// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_GunFire.generated.h"

/**
 * 
 */
UCLASS()
class NAVI_API UBTTaskNode_GunFire : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_GunFire();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	class ANaviCharacter* AICharacter;
	
};
