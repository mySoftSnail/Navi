// Fill out your copyright notice in the Description page of Project Settings.
 
#pragma once

#include "Navi/Navi.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "NaviCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Sprinting UMETA(DisplayName = "Sprinting"),
	ECS_Rolling UMETA(DisplayName = "Rolling"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),
	ECS_UsingSkill UMETA(DisplayName = "UsingSkill"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX"),
};

class USpringArmComponent;
class UCameraComponent;
class AWeapon;
class USoundCue;
class UParticleSystem;
class UAnimMontage;
class AItem;
class ANaviPlayerController;
class ANaviAIController;
class AExplosive;
class UStaticMeshComponent;
class AEnemy;
class UWidgetComponent;
class UNaviStatComponent;
class UBehaviorTree;

UCLASS()
class NAVI_API ANaviCharacter : public ACharacter
{
	GENERATED_BODY()
	
	friend class UNaviCharacterAnimInstance;
	friend class UBTService_CalcDistToTarget;
	friend class UBTTaskNode_GunFire;
	friend class UBTTaskNode_Reload;

public: 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAiming() const { return bAiming; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE USoundCue* GetMeleeImpactSound() const { return MeleeImpactSound; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
	FORCEINLINE float GetStunChance() const { return StunChance; }
	FORCEINLINE bool GetIsPlayer() const { return bIsPlayer; }
	FORCEINLINE UNaviStatComponent* GetStatComponent() const { return StatComponent; }
	FORCEINLINE AItem* GetEquippedAccessory() const { return EquippedAccessory; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE void SetIsPlayer(bool _bIsPlayer) { bIsPlayer = _bIsPlayer; }
	FORCEINLINE bool GetInBattle() const { return bInBattle; }
	FORCEINLINE void SetInBattle(bool _bInBattle) { bInBattle = _bInBattle; }
	FORCEINLINE AEnemy* GetInBattleEnemy() const { return InBattleEnemy; }
	FORCEINLINE void SetInBattleEnemy(AEnemy* _InBattleEnemy) { InBattleEnemy = _InBattleEnemy; }
	FORCEINLINE float GetAggression() const { return Aggression; }
	FORCEINLINE float GetBaseDetectRadius() const { return BaseDetectRadius; }
	FORCEINLINE void SetAIController(ANaviAIController* _AIController) { AIController = _AIController; }

public:
	ANaviCharacter(const FObjectInitializer& ObjectInitiailizer);

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void IncrementOverlappedItemCount(int8 Amount);

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	void Stun();

	UFUNCTION(BlueprintCallable)
	void AddEXP(float EXPAmount);

	void SwapItem(AItem* ItemToSwap);

	UFUNCTION(BlueprintImplementableEvent)
	void LevelUpEffect();

	void EquipItem(AItem* Item);

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void AimingButtonPressed();
	void AimingButtonReleased();
	void SkillOneButtonPressed();
	void SkillTwoButtonPressed();
	void SelectButtonPressed();
	void ReloadButtonPressed();
	void SprintOn();
	void SprintOff();
	void FireButtonPressed();
	void FireButtonReleased();
	bool WeaponHasAmmo();
	void ReloadWeapon();
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	UFUNCTION(BlueprintCallable)
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	void TraceForItems();

	void SetLookRates();

	void CameraInterpZoom(float DeltaTime);

	void CalculateCrosshairSpread(float DeltaTime);
	
	void StartCrosshairBulletFire();
	
	UFUNCTION()
	void FinishCrosshairBulletFire();

	AWeapon* SpawnDefaultWeapon();

	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();

	UFUNCTION(BlueprintCallable)
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void ControlViewMode(float DeltaTime);

	void ReadDefaultWeaponClass();

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();
	
	void AddRecoil();

	void SpawnExplosive();

	void SpawnFlare();

	void EndSkill();

	UFUNCTION(BlueprintImplementableEvent)
	void RagdollDeath();

	void UsePotion();

	UFUNCTION(BlueprintImplementableEvent)
	void PotionCoolDown();

	UFUNCTION(BlueprintImplementableEvent)
	void SkillOneCoolDown();

	UFUNCTION(BlueprintImplementableEvent)
	void SkillTwoCoolDown();

	UFUNCTION()
	void CheckInBattleEnemy();

private:
	/** 실제 유저가 조종하면 true, AI에 의해 조종되면 false */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State, meta = (AllowPrivateAccess = true))
	bool bIsPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float BaseTurnRate = 45.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float BaseLookUpRate = 45.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float HipTurnRate = 90.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float HipLookUpRate = 90.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float AimingTurnRate = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	float AimingLookUpRate = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float RunSpeed = 400.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float SprintSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bAiming = false;

	float CameraDefaultFOV = 0.f;
	float CameraZoomedFOV = 40.f;
	float CameraCurrentFOV = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor = 0.f;

	float ShootTimeDuration = 0.05;

	bool bFiringBullet = false;

	FTimerHandle CrosshairShootTimer;

	bool bFireButtonPressed = false;

	bool bShouldFire = true;

	FTimerHandle AutoFireTimer;

	bool bShouldTraceForItems = false;

	int8 OverlappedItemCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItemLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* EquippedAccessory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName DefaultWeaponItemID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	float InterpYaw = 0.f;

	bool bMoveForwardPressed = false;
	bool bMoveRightPressed = false;
	bool bSprintPressed = false;
	bool bBackViewMode = false;
	bool bSkillReadyRotationMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue* MeleeImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* SkillMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	FVector2D ForwardRightInputValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AExplosive> ExplosiveMineClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FlareGunClass;

	UPROPERTY()
	AActor* FlareGun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FlareClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Skill, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* FlareFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanUsePotion = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float PotionHealPercent = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float PotionCoolDownRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* PlayerMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* AIMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AEnemy* TraceHitEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AEnemy* LastPingEnemy;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	UNaviStatComponent* StatComponent;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	bool bCanUseSkillOne = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	float SkillOneCoolDownRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	bool bCanUseSkillTwo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill, meta = (AllowPrivateAccess = "true"))
	float SkillTwoCoolDownRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* BPTableObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HeadShotDamageFactor = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInBattle = false;

	UPROPERTY()
	AEnemy* InBattleEnemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true"))
	ANaviAIController* AIController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0",UIMin = "0.0", UIMax = "1.0"))
	float Aggression = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true"))
	float BaseDetectRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true"))
	float HeadAccuracy = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true"))
	float AIPower = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = NaviAI, meta = (AllowPrivateAccess = "true"))
	float AIMaxHP = 5000.f;
};
