// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviCharacterAnimInstance.h"
#include "NaviCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
 

UNaviCharacterAnimInstance::UNaviCharacterAnimInstance() 
{
}

void UNaviCharacterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (NaviCharacter == nullptr)
	{
		NaviCharacter = Cast<ANaviCharacter>(TryGetPawnOwner());
	}

	if (NaviCharacter)
	{
		FVector Velocity{ NaviCharacter->GetVelocity() };
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();

		if (NaviCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = NaviCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(NaviCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if (NaviCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		CharacterRotationLastFrame = CharacterRotation;
		CharacterRotation = NaviCharacter->GetActorRotation();
		const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };
		const float Target{ Delta.Yaw / DeltaTime };
		const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
		YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

		if (NaviCharacter->CombatState == ECombatState::ECS_Sprinting)
		{
			bIsSprinting = true;
		}
		else
		{
			bIsSprinting = false;
		}

		bAiming = NaviCharacter->GetAiming();
		bReloading = NaviCharacter->CombatState == ECombatState::ECS_Reloading;

		if (bAiming || bReloading)
		{
			RecoilWeight = 1.f;
		}
		else
		{
			RecoilWeight = 0.5f;
		}

		Pitch = NaviCharacter->GetBaseAimRotation().Pitch;

		NaviCharacter->bBackViewMode ? bShouldUseAimOffset = true : bShouldUseAimOffset = false;

		bShouldUseFABRIK = 
			NaviCharacter->CombatState == ECombatState::ECS_Unoccupied || 
			NaviCharacter->CombatState == ECombatState::ECS_FireTimerInProgress || 
			NaviCharacter->CombatState == ECombatState::ECS_Sprinting ||
			NaviCharacter->CombatState == ECombatState::ECS_Rolling;

		if (NaviCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = NaviCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}

}

void UNaviCharacterAnimInstance::NativeInitializeAnimation()
{
	NaviCharacter = Cast<ANaviCharacter>(TryGetPawnOwner());
}

void UNaviCharacterAnimInstance::RotateCharacter()
{
	EDirection CurrentInputDirection = GetDirectionFromInput();

	if (CurrentInputDirection == EDirection::ED_NoDirection) { return; }

	float DeltaYawAngle = GetAngleFromDirection(CurrentInputDirection);
	FRotator NewRotation(0.f, NaviCharacter->GetControlRotation().Yaw + DeltaYawAngle, 0.f);
	NaviCharacter->SetActorRotation(NewRotation);
}

EDirection UNaviCharacterAnimInstance::GetDirectionFromInput()
{
	const FVector2D ForwardRightInputValue = NaviCharacter->ForwardRightInputValue;

	if (ForwardRightInputValue.SizeSquared() < 1.f)
	{
		return EDirection::ED_NoDirection;
	}

	const float ForwardValue = ForwardRightInputValue.X;
	const float RightValue = ForwardRightInputValue.Y;

	if (ForwardValue >= 1.f)
	{
		if (RightValue >= 1.f)
		{
			// (1,1) 
			return EDirection::ED_ForwardRight;
		}
		else if (RightValue <= -1.f)
		{
			// (1,-1)
			return EDirection::ED_ForwardLeft;
		}
		else
		{
			// (1,0)
			return EDirection::ED_Forward;
		}
	}
	else if (ForwardValue <= -1.f)
	{
		if (RightValue >= 1.f)
		{
			// (-1,1)
			return EDirection::ED_BackwardRight;
		}
		else if (RightValue <= -1.f)
		{
			// (-1,-1)
			return EDirection::ED_BackwardLeft;
		}
		else
		{
			// (-1,0)
			return EDirection::ED_Backward;
		}
	}
	else
	{
		if (RightValue >= 1.f)
		{
			// (0,1)
			return EDirection::ED_Right;
		}
		else
		{
			// (0,-1)
			return EDirection::ED_Left;
		}
	}
}

float UNaviCharacterAnimInstance::GetAngleFromDirection(const EDirection& Direction)
{
	switch (Direction)
	{
	case EDirection::ED_NoDirection:
	case EDirection::ED_Forward:
		return 0.f;
		break;
	case EDirection::ED_ForwardRight:
		return 45.f;
		break;
	case EDirection::ED_ForwardLeft:
		return 315.f;
		break;
	case EDirection::ED_Right:
		return 90.f;
		break;
	case EDirection::ED_Left:
		return 270.f;
		break;
	case EDirection::ED_Backward:
		return 180.f;
		break;
	case EDirection::ED_BackwardRight:
		return 135.f;
		break;
	case EDirection::ED_BackwardLeft:
		return 225.f;
		break;
	}
	return 0.f;
}

void UNaviCharacterAnimInstance::SpawnExplosive()
{
	if (NaviCharacter == nullptr) { return; }
	NaviCharacter->SpawnExplosive();
}

void UNaviCharacterAnimInstance::SpawnFlare()
{
	if (NaviCharacter == nullptr) { return; }
	NaviCharacter->SpawnFlare();
}

void UNaviCharacterAnimInstance::EndSkill()
{
	if (NaviCharacter == nullptr) { return; }
	NaviCharacter->EndSkill();
}

void UNaviCharacterAnimInstance::EndSkillRotation()
{
	if (NaviCharacter == nullptr) { return; }
	if (NaviCharacter->bSkillReadyRotationMode)
	{
		NaviCharacter->bSkillReadyRotationMode = false;
	}
}