// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTaskNode_GunFire.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navi/Character/NaviCharacter.h"
#include "Navi/Controllers/NaviAIController.h"

UBTTaskNode_GunFire::UBTTaskNode_GunFire()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_GunFire::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AICharacter = Cast<ANaviCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (AICharacter == nullptr) { return EBTNodeResult::Failed; }

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_GunFire::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemy* TargetEnemy = Cast<AEnemy>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetEnemy")));

	if ((TargetEnemy == nullptr) || TargetEnemy->GetbDying())
	{
		AICharacter->FireButtonReleased();
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("TargetEnemy"), nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	else
	{
		FVector LookVector = TargetEnemy->GetActorLocation() - AICharacter->GetActorLocation();
		LookVector.Z = 0.f;
		FRotator TargetRot = FRotationMatrix::MakeFromX(LookVector).Rotator();
		AICharacter->SetActorRotation(FMath::RInterpTo(AICharacter->GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 30.0f));

		AICharacter->FireButtonPressed();
	}

	if (AICharacter->WeaponHasAmmo())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		AICharacter->FireButtonReleased();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}