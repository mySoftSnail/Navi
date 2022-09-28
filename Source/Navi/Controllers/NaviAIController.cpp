// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navi/Character/NaviCharacter.h"
#include "Navi/Components/NaviStatComponent.h"

ANaviAIController::ANaviAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ANaviAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ANaviCharacter* NaviChar = Cast<ANaviCharacter>(InPawn);
	if (NaviChar)
	{
		NaviChar->SetIsPlayer(false);
		NaviChar->SetAIController(this);

		check(NaviChar->GetBehaviorTree());
		BlackboardComponent->InitializeBlackboard(*(NaviChar->GetBehaviorTree()->BlackboardAsset));
		BlackboardComponent->SetValueAsBool(TEXT("ShouldAttack"), false);
		BlackboardComponent->SetValueAsBool(TEXT("IsDead"), false);
	}
}

void ANaviAIController::BeginPlay()
{
	Super::BeginPlay();

	// OnPosess -> BeginPlay
	ANaviCharacter* NaviPlayer = Cast<ANaviCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	check(NaviPlayer);
	BlackboardComponent->SetValueAsObject(TEXT("NaviPlayer"), NaviPlayer);
	NaviPlayer->SetAIController(this);

	RunBehaviorTree(NaviPlayer->GetBehaviorTree());
}
