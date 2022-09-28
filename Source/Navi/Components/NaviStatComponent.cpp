// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviStatComponent.h"

#include "Navi/Character/NaviCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navi/GameInstance/NaviGameInstance.h"

// Sets default values for this component's properties
UNaviStatComponent::UNaviStatComponent() :
	Level(0),
	MaxHP(1),
	NextEXP(1)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNaviStatComponent::ChangeHP(float HPAmount)
{
	CurrentHP = FMath::Clamp<float>(CurrentHP + HPAmount, 0.0f, MaxHP);
}

void UNaviStatComponent::LevelUp()
{
	if (Level <= 0)
	{
		// 스탯 기본값 세팅 (레벨 1의 상태)
		++Level; 
		MaxHP = 100.f;
		CurrentHP = MaxHP;
		NextEXP = 200.f;
		Power = 0.f;
	}
	else if (Level == 10)
	{
		CurrentEXP = 0.f;
	}	
	else
	{
		++Level;

		MaxHP = MaxHP + 50.f; // 레벨업 시 HP 증가량

		CurrentHP = MaxHP; // 레벨업시 체력 회복 (옵션)

		++SkillPoint;
		
		NextEXP += NextEXP * 0.4f; // 요구 경험치 40%씩 증가

		if (bShouldPlayLevelUpEffect)
		{
			ANaviCharacter* NaviCharacter = Cast<ANaviCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			NaviCharacter->LevelUpEffect();
		}
	}
}

void UNaviStatComponent::AddEXP(float EXPAmount)
{
	if(Level != 10)
	{
		if (bShouldPlayLevelUpEffect == false) { bShouldPlayLevelUpEffect = true; }

		float LocalEXP = CurrentEXP + EXPAmount;

		if (LocalEXP >= NextEXP)
		{
			while (LocalEXP >= NextEXP)
			{
				CurrentEXP = LocalEXP - NextEXP;
				LocalEXP = CurrentEXP;
				LevelUp();
			}
		}
		else
		{
			CurrentEXP = LocalEXP;
		}		
	}
	// DB에 스탯 업로드
	UploadStat();
}

void UNaviStatComponent::UploadStat()
{
	FNaviStat NewStat;
	NewStat.Level = Level;
	NewStat.EXP = CurrentEXP;
	NewStat.SkillPoint = SkillPoint;
	NewStat.SkillOneLevel = SkillOneLevel;
	NewStat.SkillTwoLevel = SkillTwoLevel;

	auto NaviGameInstance = Cast<UNaviGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	NaviGameInstance->NaviStat = NewStat;
	NaviGameInstance->UploadStat();
}

void UNaviStatComponent::SkillLevelUp(int32 SkillNumber)
{
	if (SkillPoint <= 0) { return; }

	switch (SkillNumber)
	{
	case 1:
		if ((SkillOneLevel + 1) < 4)
		{
			SkillOneLevel++;
			SkillPoint--;
		}
		break;
	case 2:
		if ((SkillTwoLevel + 1) < 4)
		{
			SkillTwoLevel++;
			SkillPoint--;
		}
		break;
	}

	UploadStat();
}

void UNaviStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNaviStatComponent::InitPlayerStat()
{
	FNaviStat InitStat = Cast<UNaviGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->NaviStat;

	while (Level < InitStat.Level)
	{
		LevelUp();
	}

	SkillPoint = InitStat.SkillPoint;
	SkillOneLevel = InitStat.SkillOneLevel;
	SkillTwoLevel = InitStat.SkillTwoLevel;
	CurrentEXP = InitStat.EXP;
}