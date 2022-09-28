// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_Reload.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navi/Character/NaviCharacter.h"
#include "Navi/Controllers/NaviAIController.h"

UBTTaskNode_Reload::UBTTaskNode_Reload()
{
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_Reload::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AICharacter = Cast<ANaviCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (AICharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	if (AICharacter->WeaponHasAmmo() == false)
	{
		AICharacter->ReloadWeapon();
	}

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_Reload::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (AICharacter->WeaponHasAmmo() == false)
	{
		AICharacter->ReloadWeapon();
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
