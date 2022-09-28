// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NaviCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class NAVI_API UNaviCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UNaviCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

};
