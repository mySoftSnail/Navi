// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define EPS_Rock EPhysicalSurface::SurfaceType1
#define EPS_Sand EPhysicalSurface::SurfaceType2
#define EPS_Wetness EPhysicalSurface::SurfaceType3


/** 8방향과 무방향을 표현하는 Enum */
UENUM(BlueprintType)
enum class EDirection : uint8
{
	ED_NoDirection UMETA(DisplayName = "NoDirection"),
	ED_Forward UMETA(DisplayName = "Forward"),
	ED_ForwardRight UMETA(DisplayName = "ForwardRight"),
	ED_ForwardLeft UMETA(DisplayName = "ForwardLeft"),
	ED_Right UMETA(DisplayName = "Right"),
	ED_Left UMETA(DisplayName = "Left"),
	ED_Backward UMETA(DisplayName = "Backward"),
	ED_BackwardRight UMETA(DisplayName = "BackwardRight"),
	ED_BackwardLeft UMETA(DisplayName = "BackwardLeft"),
};

