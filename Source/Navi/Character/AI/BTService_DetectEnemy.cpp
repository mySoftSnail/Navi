// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_DetectEnemy.h"
#include "Navi/Controllers/NaviAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Navi/Enemy/Enemy.h"
#include "Navi/Character/NaviCharacter.h"

UBTService_DetectEnemy::UBTService_DetectEnemy()
{
	NodeName = TEXT("Detect Enemy");
	Interval = 0.2f;
}

void UBTService_DetectEnemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* TargetEnemy = Cast<AEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetEnemy")));

	if (TargetEnemy == nullptr)
	{
		AICharacter = AICharacter == nullptr ? Cast<ANaviCharacter>(OwnerComp.GetAIOwner()->GetPawn()) : AICharacter;
		Player = Player == nullptr ? Cast<ANaviCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("NaviPlayer"))) : Player;
		if (AICharacter == nullptr || Player == nullptr) return;
		UWorld* World = AICharacter->GetWorld();
		if (World == nullptr) return;

		float DetectRadius = AICharacter->GetBaseDetectRadius() * AICharacter->GetAggression();
		FVector Center = AICharacter->GetActorLocation();

		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams CollisionQueryParam(NAME_None, false, AICharacter);
		CollisionQueryParam.AddIgnoredActor(Player);

		bool bResult = World->OverlapMultiByChannel(
			OverlapResults,
			Center,
			FQuat::Identity,
			ECC_EnemyDetection,
			FCollisionShape::MakeSphere(DetectRadius),
			CollisionQueryParam
		);

		FColor Color = FColor::Red;
		bool bDetect = false;
		if (bResult)
		{
			Color = FColor::Green;
			for (auto const& OverlapResult : OverlapResults)
			{
				AEnemy* DetectedEnemy = Cast<AEnemy>(OverlapResult.GetActor());
				if (DetectedEnemy)
				{
					if (DetectedEnemy->GetbDying()) { continue; }

					bDetect = true;
					OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("DetectedEnemy"), DetectedEnemy);
					break;
				}
			}
		}
		if (DrawDebug) DrawDebugSphere(World, Center, DetectRadius, 32, Color, false, 0.2f);
		if (!bDetect)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("DetectedEnemy"), nullptr);
		}
	}
}