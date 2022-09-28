// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Navi/Navi.h"
#include "Animation/AnimInstance.h"
#include "Navi/Items/Weapon.h"
#include "NaviCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NAVI_API UNaviCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	friend class ANaviCharacter;
	
public:
	UNaviCharacterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable)
	void RotateCharacter();

	EDirection GetDirectionFromInput();

	/** (1,0)인 벡터와 입력 값이 이루는 각도를 반환하는 함수 */
	float GetAngleFromDirection(const EDirection& Direction);

	UFUNCTION(BlueprintCallable)
	void SpawnExplosive();

	UFUNCTION(BlueprintCallable)
	void SpawnFlare();

	UFUNCTION(BlueprintCallable)
	void EndSkill();

	UFUNCTION(BlueprintCallable)
	void EndSkillRotation();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class ANaviCharacter* NaviCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDelta = 0.f;

	UPROPERTY()
	FRotator CharacterRotation;

	UPROPERTY()
	FRotator CharacterRotationLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bReloading = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	float Pitch = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseAimOffset = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType = EWeaponType::EWT_MAX;

};
