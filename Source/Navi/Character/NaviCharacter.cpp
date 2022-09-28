// Fill out your copyright notice in the Description page of Project Settings.

#include "NaviCharacter.h"
#include "ChaosInterfaceWrapperCore.h"
#include "Navi/Enemy/Enemy.h"
#include "Navi/Enemy/EnemyAIController.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NaviCharacterAnimInstance.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Navi/Items/Weapon.h"
#include "Navi/Items/Item.h"
#include "Components/WidgetComponent.h"
#include "Navi/Controllers/NaviPlayerController.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Navi/Interfaces/HitInterface.h"
#include "Navi/Controllers/NaviAIController.h"
#include "Navi/Components/NaviCharacterMovementComponent.h"
#include "Navi/GameMode/NaviGameModeBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navi/LevelObjects/Explosive.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Navi/Components/NaviStatComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"

ANaviCharacter::ANaviCharacter(const FObjectInitializer& ObjectInitiailizer):
	Super(ObjectInitiailizer.SetDefaultSubobjectClass<UNaviCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->bUsePawnControlRotation = false; 
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
	StatComponent = CreateDefaultSubobject<UNaviStatComponent>(TEXT("StatComponent"));
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 플레이어가 조종하는 캐릭터를 제외한 레벨에 배치된 모든 캐릭터는 AI Controller에 빙의
	AIControllerClass = ANaviAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
}

void ANaviCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	if (bIsPlayer)
	{
		GetMesh()->SetSkeletalMesh(PlayerMesh);
	}
	else
	{
		GetMesh()->SetSkeletalMesh(AIMesh);
		EquipItem(SpawnDefaultWeapon());

		StatComponent->SetPower(AIPower);
		StatComponent->SetMaxHP(AIMaxHP);
		StatComponent->SetCurrentHP(AIMaxHP);
	}

	InterpYaw = GetActorRotation().Yaw;
}

void ANaviCharacter::MoveForward(float Value)
{
	if (CombatState == ECombatState::ECS_UsingSkill) { return; }

	ForwardRightInputValue.X = Value;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		bMoveForwardPressed = true;
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
	else { bMoveForwardPressed = false; }
}

void ANaviCharacter::MoveRight(float Value)
{
	if (CombatState == ECombatState::ECS_UsingSkill) { return; }

	ForwardRightInputValue.Y = Value;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		bMoveRightPressed = true;
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
	else { bMoveRightPressed = false; }
}

void ANaviCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * 0.6f); 
}

void ANaviCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * 0.6f);
}

void ANaviCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();
		AddRecoil();
		StartFireTimer();
	}		
	
	StartCrosshairBulletFire();
}

bool ANaviCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;

	if (bIsPlayer)
	{
		FHitResult CrosshairHitResult;
		bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

		if (bCrosshairHit)
		{
			OutBeamLocation = CrosshairHitResult.Location;
		}
	}
	else // AI 동료 캐릭터의 일반 공격 Trace
	{
		AEnemy* TargetEnemy = Cast<AEnemy>(AIController->BlackboardComponent->GetValueAsObject(TEXT("TargetEnemy")));

		float HeadShotFactor = FMath::RandRange(0.f, 1.f);
		if (HeadAccuracy >= HeadShotFactor)
		{
			OutBeamLocation = TargetEnemy->GetMesh()->GetBoneLocation(FName(TargetEnemy->GetHeadBone()));
		}
		else
		{
			OutBeamLocation = TargetEnemy->GetActorLocation();
		}
	}
	
	// Perform a second trace, this time from the gun barrel
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (OutHitResult.bBlockingHit == false)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;	
}

void ANaviCharacter::SprintOn()
{
	if (CombatState != ECombatState::ECS_Unoccupied) { return; }
	CombatState = ECombatState::ECS_Sprinting;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	bSprintPressed = true;
}

void ANaviCharacter::SprintOff()
{
	if(CombatState != ECombatState::ECS_Sprinting) { return; }
	CombatState = ECombatState::ECS_Unoccupied;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	bSprintPressed = false;
}

void ANaviCharacter::FireButtonPressed()
{
	if (CombatState == ECombatState::ECS_Sprinting) { SprintOff(); }

	bFireButtonPressed = true;
	FireWeapon();
}

void ANaviCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void ANaviCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(
		AutoFireTimer,
		this,
		&ANaviCharacter::AutoFireReset,
		EquippedWeapon->GetAutoFireRate());
}

void ANaviCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic()) { FireWeapon(); }
	}
	else { ReloadWeapon(); }
}

bool ANaviCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// Trace from Crosshair world location outward
		
		const FVector Start{ CrosshairWorldPosition };
		const FVector End { Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}	
	return false;
}

void ANaviCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}
			
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

void ANaviCharacter::AimingButtonPressed()
{
	bAiming = true;
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping && CombatState != ECombatState::ECS_Stunned)
	{
		GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
	}	
}

void ANaviCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void ANaviCharacter::SkillOneButtonPressed()
{
	if (CombatState != ECombatState::ECS_UsingSkill)
	{
		if (bIsPlayer && bCanUseSkillOne)
		{
			UAnimInstance* Anim = GetMesh()->GetAnimInstance();
			if (Anim && SkillMontage)
			{
				Anim->Montage_Play(SkillMontage);
				Anim->Montage_JumpToSection(FName("MineSkill"));

				CombatState = ECombatState::ECS_UsingSkill;
				bSkillReadyRotationMode = true;

				bCanUseSkillOne = false;
				SkillOneCoolDownRate = 0.f;
				SkillOneCoolDown();
			}
		}
	}
}

void ANaviCharacter::SkillTwoButtonPressed()
{
	if (CombatState != ECombatState::ECS_UsingSkill)
	{
		if (bIsPlayer && bCanUseSkillTwo)
		{
			UAnimInstance* Anim = GetMesh()->GetAnimInstance();
			if (Anim && SkillMontage)
			{
				CombatState = ECombatState::ECS_UsingSkill;
				
				Anim->Montage_Play(SkillMontage);
				Anim->Montage_JumpToSection(FName("BombingSkill"));

				bSkillReadyRotationMode = true;

				// 주무기 잠깐 숨기기
				EquippedWeapon->SetActorHiddenInGame(true);

				const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

				FlareGun = GetWorld()->SpawnActor<AActor>(FlareGunClass);
				FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, false);
				FlareGun->AttachToComponent(GetMesh(), Rules, TEXT("RightHandSocket"));

				bCanUseSkillTwo = false;				
				SkillTwoCoolDownRate = 0.f;
				SkillTwoCoolDown();
			}
		}
	}
}

void ANaviCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void ANaviCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void ANaviCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size());

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}
	
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void ANaviCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ANaviCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void ANaviCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

AWeapon* ANaviCharacter::SpawnDefaultWeapon()
{
	ReadDefaultWeaponClass();

	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void ANaviCharacter::EquipItem(AItem* Item)
{
	if (Item == nullptr) { return; }
	
	AWeapon* WeaponToEquip = Cast<AWeapon>(Item);
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
	else
	{
		EquippedAccessory = Item;
	}

	StatComponent->SetPower(StatComponent->GetPower() + Item->GetItemPower());
	StatComponent->SetMaxHP(StatComponent->GetMaxHP() + Item->GetItemMaxHP());
}

void ANaviCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		if (TraceHitItem->GetPickupSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}

		ANaviPlayerController* NaviPlayerController = Cast<ANaviPlayerController>(GetController());

		NaviPlayerController->PickupItem(TraceHitItem);

		TraceHitItem = nullptr;
		TraceHitItemLastFrame = nullptr;
	}
}

void ANaviCharacter::SwapItem(AItem* ItemToSwap)
{
	if (ItemToSwap == nullptr) { return; }

	AWeapon* WeaponToSwap = Cast<AWeapon>(ItemToSwap);
	if (WeaponToSwap)
	{
		StatComponent->SetPower(StatComponent->GetPower() - EquippedWeapon->GetItemPower());
		EquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	}
	else
	{
		if (EquippedAccessory)
		{
			StatComponent->SetPower(StatComponent->GetPower() - EquippedAccessory->GetItemPower());
			StatComponent->SetMaxHP(StatComponent->GetMaxHP() - EquippedAccessory->GetItemMaxHP());
		}
	}

	EquipItem(ItemToSwap);
}

bool ANaviCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void ANaviCharacter::PlayFireSound()
{
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void ANaviCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");

	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}

		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			if (BeamHitResult.Actor.IsValid())
			{
				IHitInterface* HitInterface = Cast<IHitInterface>(BeamHitResult.Actor.Get());
				if (HitInterface)
				{
					HitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}
				else
				{
					if (ImpactParticles) // Spawn default particels
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
					}
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if (HitEnemy)
				{
					bool bHitHead = BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone();
					float Damage = EquippedWeapon->GetDamage() * StatComponent->GetPower();
					if (bHitHead) { Damage *= HeadShotDamageFactor; }

					UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage, GetController(), this, UDamageType::StaticClass());

					if (bIsPlayer && HitEnemy->GetbDying() == false) 
					{ 
						HitEnemy->ShowHitMarker(BeamHitResult.Location, bHitHead); 
						bInBattle = true;
						InBattleEnemy = HitEnemy;
					}
				}
			}

			// Smoke Trail 
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}

void ANaviCharacter::PlayGunFireMontage()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (Anim && HipFireMontage)
	{
		Anim->Montage_Play(HipFireMontage);
		Anim->Montage_JumpToSection(FName("StartFire"));
	}
}

void ANaviCharacter::ReloadButtonPressed()
{
	if (EquippedWeapon->GetAmmo() != EquippedWeapon->GetMagazineCapacity())
	{
		ReloadWeapon();	
	}	
}

void ANaviCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (Anim && ReloadMontage)
	{
		Anim->Montage_Play(ReloadMontage);
		Anim->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
	}
}

void ANaviCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void ANaviCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void ANaviCharacter::ReadDefaultWeaponClass()
{
	if (BPTableObject)
	{
		/** RowName과 ItemID가 일치하도록 직접 지정할 것 */
		FItemBPTable* ItemBPRow = nullptr;
		ItemBPRow = BPTableObject->FindRow<FItemBPTable>(DefaultWeaponItemID, TEXT(""));
		
		if (ItemBPRow)
		{
			DefaultWeaponClass = ItemBPRow->ItemClass;
		}
	}
}

EPhysicalSurface ANaviCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector(0.0f, 0.0f, -400.f) };

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true; 

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void ANaviCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void ANaviCharacter::ControlViewMode(float DeltaTime)
{
	bBackViewMode = bMoveForwardPressed || bMoveRightPressed || bFireButtonPressed || bAiming;

	bBackViewMode = bBackViewMode && (bSprintPressed == false);

	bBackViewMode = bBackViewMode && (CombatState != ECombatState::ECS_Rolling);

	if (bBackViewMode || bSkillReadyRotationMode)
	{
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), GetControlRotation(), DeltaTime, 15.f);
		NewRotation.Pitch = 0.f;
		NewRotation.Roll = 0.f;

		SetActorRotation(NewRotation);

		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void ANaviCharacter::UsePotion()
{
	if (bCanUsePotion == true && StatComponent->GetCurrentHP() < StatComponent->GetMaxHP())
	{
		bCanUsePotion = false;

		StatComponent->SetCurrentHP(FMath::Clamp(StatComponent->GetCurrentHP() + (PotionHealPercent / 100 * StatComponent->GetMaxHP()), 0.f, StatComponent->GetMaxHP()));
		PotionCoolDownRate = 0.f;
		PotionCoolDown();
	}	
}

void ANaviCharacter::CheckInBattleEnemy()
{
	if (InBattleEnemy)
	{
		if (InBattleEnemy->GetbDying() || !::IsValid(InBattleEnemy))
		{
			InBattleEnemy = nullptr;
			bInBattle = false;
		}
	}
}

void ANaviCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	SetLookRates();
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
	ControlViewMode(DeltaTime);
	CheckInBattleEnemy();
}	

void ANaviCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANaviCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANaviCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ANaviCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ANaviCharacter::LookUp);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANaviCharacter::SprintOn);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANaviCharacter::SprintOff);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ANaviCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &ANaviCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &ANaviCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &ANaviCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ANaviCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &ANaviCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("SkillOne", IE_Pressed, this, &ANaviCharacter::SkillOneButtonPressed);
	PlayerInputComponent->BindAction("SkillTwo", IE_Pressed, this, &ANaviCharacter::SkillTwoButtonPressed);
	PlayerInputComponent->BindAction("Potion", IE_Pressed, this, &ANaviCharacter::UsePotion);
}

void ANaviCharacter::AddEXP(float EXPAmount)
{
	if (bIsPlayer)
	{
		StatComponent->AddEXP(EXPAmount);
	}
	else // 동료가 처치
	{
		Cast<ANaviCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->AddEXP(EXPAmount);
	}
}

void ANaviCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;

	EquippedWeapon->ReloadAmmo();
}

float ANaviCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void ANaviCharacter::Stun()
{
	if (StatComponent->GetCurrentHP() <= 0.f) return;
	
	if (CombatState != ECombatState::ECS_UsingSkill) // 스킬 시전 중에는 스턴 당하지 않음
	{
		CombatState = ECombatState::ECS_Stunned;

		UAnimInstance* Anim = GetMesh()->GetAnimInstance();
		if (Anim && HitReactMontage)
		{
			Anim->Montage_Play(HitReactMontage);
		}
	}
}

float ANaviCharacter::TakeDamage(float DamageAmount, FDamageEvent const& MovieSceneBlends, AController* EventInstigator, AActor* DamageCauser)
{
	if (StatComponent->GetCurrentHP() - DamageAmount <= 0.f)
	{
		StatComponent->SetCurrentHP(0.f);
		Die();

		auto EnemyAIController = Cast<AEnemyAIController>(EventInstigator);
		if (EnemyAIController)
		{
			EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("CharacterDead")), true);
		}
	}
	else
	{
		StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - DamageAmount);
	}
	return DamageAmount;
}

void ANaviCharacter::Die()
{
	FinishDeath();
}

void ANaviCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	if (bIsPlayer)
	{
		ANaviPlayerController* NaviPlayerController = Cast<ANaviPlayerController>(GetController());
		DisableInput(NaviPlayerController);
		NaviPlayerController->OnPlayerDieDelegate.Broadcast();
	}
	else
	{
		AIController->GetBlackboardComponent()->SetValueAsBool (TEXT("IsDead"), true);
	}

	RagdollDeath();
}

void ANaviCharacter::AddRecoil()
{
	if (EquippedWeapon == nullptr) { return; }

	float PitchValue = -0.1f;
	float YawValue = FMath::RandBool() ? 0.05f : -0.05f;

	// 1. 무기 고유의 반동 정도 
	PitchValue *= EquippedWeapon->GetRecoilValue();
	YawValue *= EquippedWeapon->GetRecoilValue();

	// 2. Aiming 상태 - 조준 시 반동 60% 감소
	if (bAiming)
	{
		PitchValue *= 0.4f;
		YawValue *= 0.4f;
	}

	AddControllerPitchInput(PitchValue);
	AddControllerYawInput(YawValue);
}

void ANaviCharacter::SpawnExplosive()
{
	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("LeftHandSocket"));

	if (ExplosiveMineClass)
	{
		AExplosive* MineActor = GetWorld()->SpawnActor<AExplosive>(ExplosiveMineClass, HandSocket->GetSocketLocation(GetMesh()), FRotator(0.f, GetActorRotation().Yaw, 0.f));
		int32 SkillLevel = StatComponent->SkillOneLevel;
		float BaseDamage = SkillLevel * 15.0f;
		float FinalDamage = BaseDamage * StatComponent->GetPower();
		MineActor->SetDamage(FinalDamage);
	}
}

void ANaviCharacter::SpawnFlare()
{
	const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	GetWorld()->SpawnActor<AActor>(FlareClass, HandSocket->GetSocketLocation(GetMesh()), FRotator(0.f, GetActorRotation().Yaw, 0.f));
}

void ANaviCharacter::EndSkill()
{
	if (CombatState != ECombatState::ECS_UsingSkill) { return; }

	CombatState = ECombatState::ECS_Unoccupied;

	// 폭격 요청 스킬에서 숨겨진 무기가 다시 보이게 한다
	EquippedWeapon->SetActorHiddenInGame(false);

	if (FlareGun)
	{
		FlareGun->Destroy();
	}
}

void ANaviCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}