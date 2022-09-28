// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CalcDistToTarget.generated.h"

class ANaviCharacter;

/**
 * 
 */
UCLASS()
class NAVI_API UBTService_CalcDistToTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CalcDistToTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	ANaviCharacter* Player;

	UPROPERTY()
	ANaviCharacter* AICharacter;
};
