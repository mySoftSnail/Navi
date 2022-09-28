// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CalcDistToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navi/Character/NaviCharacter.h"
#include "Navi/Enemy/Enemy.h"

UBTService_CalcDistToTarget::UBTService_CalcDistToTarget()
{
	NodeName = TEXT("Calculate Distance to Target and Run or Walk");
	Interval = 0.2f;
}

void UBTService_CalcDistToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AICharacter = AICharacter == nullptr ? Cast<ANaviCharacter>(OwnerComp.GetAIOwner()->GetPawn()) : AICharacter;
	if (AICharacter)
	{
		bool bShouldAttack = OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("ShouldAttack"));

		float Distance = 0.f;
		if (bShouldAttack)
		{
			AEnemy* TargetEnemy = Cast<AEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetEnemy")));
			if (TargetEnemy)
			{
				Distance = AICharacter->GetDistanceTo(TargetEnemy);
			}
		}
		else
		{
			Player = Player == nullptr ? Cast<ANaviCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("NaviPlayer"))) : Player;
			Distance = AICharacter->GetDistanceTo(Player);
		}

		if (Distance > 500.f)
		{
			AICharacter->SprintOn();
		}
		else
		{
			AICharacter->SprintOff();
		}
	}
}