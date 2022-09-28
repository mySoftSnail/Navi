// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NaviStatComponent.generated.h"

USTRUCT(BlueprintType)
struct FNaviStat
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillOneLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillTwoLevel;
};

class ANaviCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NAVI_API UNaviStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNaviStatComponent();

	FORCEINLINE int32 GetLevel() const { return Level; }
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
	FORCEINLINE float GetMaxHP() const { return MaxHP; }
	FORCEINLINE float GetPower() const { return Power; }
	FORCEINLINE float GetNextEXP() const { return NextEXP; }
	FORCEINLINE float GetCurrentEXP() const { return CurrentEXP; }
	FORCEINLINE int32 GetSkillPoint() const { return SkillPoint; }
	FORCEINLINE int32 GetSkillOneLevel() const { return SkillOneLevel; }
	FORCEINLINE int32 GetSkillTwoLevel() const { return SkillOneLevel; }

	FORCEINLINE void SetCurrentHP(float NewCurrentHP) { CurrentHP = NewCurrentHP; }
	FORCEINLINE void SetMaxHP(float NewMaxHP) { MaxHP = NewMaxHP; }
	FORCEINLINE void SetPower(float NewPower) { Power = ((NewPower >= 0.f) ? NewPower : 0.f); }

	void ChangeHP(float HPAmount);
	void LevelUp();
	void AddEXP(float EXPAmount);

	void UploadStat();

	UFUNCTION(BlueprintCallable)
	void SkillLevelUp(int32 SkillNumber);

	void InitPlayerStat();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	int32 Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	float CurrentHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	float MaxHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	float Power;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	float NextEXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	float CurrentEXP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	int32 SkillPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	int32 SkillOneLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NaviStatComponent", meta = (AllowPrivateAccess = "true"))
	int32 SkillTwoLevel;
	
	bool bShouldPlayLevelUpEffect = false;
};
