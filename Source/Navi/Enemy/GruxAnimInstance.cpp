// Fill out your copyright notice in the Description page of Project Settings.


#include "GruxAnimInstance.h"
#include "Enemy.h"

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	Enemy = Enemy == nullptr ? Cast<AEnemy>(TryGetPawnOwner()) : Enemy;

	if (Enemy)
	{
		FVector Velocity{ Enemy->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
