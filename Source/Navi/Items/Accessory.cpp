// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessory.h"

AAccessory::AAccessory()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAccessory::BeginPlay()
{
	Super::BeginPlay();
}

void AAccessory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
