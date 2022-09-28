// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Accessory.generated.h"

/**
 * 
 */
UCLASS()
class NAVI_API AAccessory : public AItem
{
	GENERATED_BODY()
	
public:
	AAccessory();

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
