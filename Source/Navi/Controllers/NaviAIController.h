// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navi/Enemy/Enemy.h"
#include "NaviAIController.generated.h"

/**
 * 
 */
UCLASS()
class NAVI_API ANaviAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
	
public:
	ANaviAIController();

	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
public:
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

private:
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

};
