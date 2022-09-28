// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyAIController.h"
#include "Navi/Character/NaviCharacter.h"
#include "Components/SphereComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navi/GameMode/NaviGameModeBase.h"

AEnemy::AEnemy() 
{
	PrimaryActorTick.bCanEverTick = true;
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
	DeactivateLeftWeapon();
	DeactivateRightWeapon();

	EnemyAIController = Cast<AEnemyAIController>(GetController());

	if (EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}

	const FVector WorldPatrolPointOne = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPointOne);
	const FVector WorldPatrolPointTwo = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPointTwo);

	if (EnemyAIController)
	{		
		EnemyAIController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPointOne"), WorldPatrolPointOne);
		EnemyAIController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPointTwo"), WorldPatrolPointTwo);
		EnemyAIController->RunBehaviorTree(BehaviorTree);
	}

	Health = MaxHealth;
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	if (bDying) return;
	bDying = true;

	HideHealthBar();
	ShowDeathMarker(bDying);
	
	Cast<ANaviCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->AddEXP(EnemyDropExp);

	if (EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		EnemyAIController->StopMovement();
	}

	DeactivateLeftWeapon();
	DeactivateRightWeapon();

	if (bIsBoss)
	{
		OnBossDieDelegate.Broadcast();
	}

	FinishDeath();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
	}	
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitMarker(UUserWidget* HitMarker, FVector Location)
{
	HitMarkers.Add(HitMarker, Location);
	FTimerHandle HitMarkerTimer;
	FTimerDelegate HitMarkerDelegate;
	HitMarkerDelegate.BindUFunction(this, FName("DestroyHitMarker"), HitMarker);
	GetWorld()->GetTimerManager().SetTimer(HitMarkerTimer, HitMarkerDelegate, HitMarkerDestroyTime, false);
}

void AEnemy::DestroyHitMarker(UUserWidget* HitMarker)
{
	HitMarkers.Remove(HitMarker);
	HitMarker->RemoveFromParent();
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	
	auto Character = Cast<ANaviCharacter>(OtherActor);
	if (Character && EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
	}
	
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;

	if (EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	
	auto NaviCharacter = Cast<ANaviCharacter>(OtherActor);
	if (NaviCharacter)
	{
		bInAttackRange = true;
	
		if (EnemyAIController)
		{
			EnemyAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}	
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	
	auto NaviCharacter = Cast<ANaviCharacter>(OtherActor);
	if (NaviCharacter)
	{
		bInAttackRange = false;

		if (EnemyAIController)
		{
			EnemyAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}	
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	FName SectionName;
	const int32 Section{ FMath::RandRange(1, 4) };
	switch (Section)
	{
	case 1:
		SectionName = TEXT("AttackLFast");
		break;
	case 2:
		SectionName = TEXT("AttackRFast");
		break;
	case 3:
		SectionName = TEXT("AttackL");
		break;
	case 4:
		SectionName = TEXT("AttackR");
		break;
	}
	return SectionName;
}

void AEnemy::DoDamage(ANaviCharacter* Victim)
{
	if (Victim == nullptr) return;

	UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyAIController, this, UDamageType::StaticClass());

	if (Victim->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Victim->GetMeleeImpactSound(), GetActorLocation());
	}
}

void AEnemy::SpawnBlood(ANaviCharacter* Victim, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };
	if (TipSocket)
	{
		const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
		if (Victim->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Victim->GetBloodParticles(), SocketTransform);
		}
	}
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Character = Cast<ANaviCharacter>(OtherActor);
	if (Character && Character->CanBeDamaged())
	{
		DoDamage(Character);
		SpawnBlood(Character, LeftWeaponSocket);
		StunCharacter(Character);
	}	
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Character = Cast<ANaviCharacter>(OtherActor);
	if (Character && Character->CanBeDamaged())
	{
		DoDamage(Character);
		SpawnBlood(Character, RightWeaponSocket);
		StunCharacter(Character);
	}	
}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::StunCharacter(ANaviCharacter* Victim)
{
	if (Victim)
	{
		const float Stun { FMath::FRandRange(0.f, 1.f) };
		if (Stun <= Victim->GetStunChance())
		{
			Victim->Stun();
		}
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	RagdollDeath();

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);	
}

void AEnemy::DestroyEnemy()
{
	EnemyDestroy();
}

void AEnemy::Stun()
{
	PlayHitMontage(FName("HitReactFront"));
	SetStunned(true);
}

void AEnemy::StartSlow(float Dilation)
{
	CustomTimeDilation = Dilation;
}

void AEnemy::EndSlow()
{
	if (CustomTimeDilation < 1.0f)
	{
		CustomTimeDilation = 1.0f;
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* NaviChar, AController* NaviCon)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,	AActor* DamageCauser)
{
	ShowHealthBar();
	
	if (EnemyAIController)
	{
		// 캐릭터가 일반 공격으로 대미지를 입힌 경우
		if (Cast<ANaviCharacter>(DamageCauser))
		{
			// 총을 쏜 대상을 타겟으로 지정
			EnemyAIController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
		}
		// 스킬 액터(지뢰, 미사일 등)에 의해 대미지를 입은 경우
		else
		{
			// 유저 플레이어 캐릭터를 타겟으로 지정
			EnemyAIController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		}
	}
	
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}

	if(!bDying)
	{
		const float Stunned = FMath::FRandRange(0.f, 1.f);
		if (Stunned <= StunChance)
		{
			Stun();
		}
	}

	return DamageAmount;
}

void AEnemy::TakeStunAttack(float StunTime, float Dilation)
{
	Stun();

	// 피격당한 에너미 움직임 느려짐
	CustomTimeDilation = Dilation;

	// 입력으로 받은 시간이 지난 뒤 원래 속도로 설정
	FTimerHandle StunTimer;

	GetWorld()->GetTimerManager().SetTimer(StunTimer, FTimerDelegate::CreateLambda([&]()
		{
			CustomTimeDilation = 1.f;
		}), StunTime, false);
}