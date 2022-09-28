// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviCharacterMovementComponent.h"

UNaviCharacterMovementComponent::UNaviCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bUseAccelerationForPaths = true;
}
