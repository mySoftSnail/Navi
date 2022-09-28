// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PollBattleState.h"
#include "Navi/Character/NaviCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navi/Enemy/Enemy.h"

UBTService_PollBattleState::UBTService_PollBattleState()
{
	NodeName = TEXT("Check the Battle State of the Player");
	Interval = 0.2f;
}

void UBTService_PollBattleState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	Player = Player == nullptr ? Cast<ANaviCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("NaviPlayer"))) : Player;

	if (Player)
	{
		auto DetectedEnemy = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("DetectedEnemy"));
		if (Player->GetInBattle() || DetectedEnemy)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("ShouldAttack"), true);

			if (Player->GetInBattle() && Player->GetInBattleEnemy())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("TargetEnemy"), Player->GetInBattleEnemy());
			}
			else if (DetectedEnemy)
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("TargetEnemy"), DetectedEnemy);
			}
		}
		else
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("ShouldAttack"), false);
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("TargetEnemy"), nullptr);
		}
	}
}
