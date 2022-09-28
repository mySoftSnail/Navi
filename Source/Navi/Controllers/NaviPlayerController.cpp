// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviPlayerController.h"
#include "Navi/Items/Item.h"
#include "Blueprint/UserWidget.h"
#include "Navi/GameMode/NaviGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "NaviAIController.h"
#include "Navi/Items/Weapon.h"
#include "Navi/Character/NaviCharacter.h"
#include "Navi/GameInstance/NaviGameInstance.h"
#include "Navi/Components/NaviStatComponent.h"

ANaviPlayerController::ANaviPlayerController() 
{
	Inventory.Init(nullptr, INVENTORY_CAPACITY);
}

void ANaviPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ANaviPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

	ANaviCharacter* NaviChar = Cast<ANaviCharacter>(InPawn);
	if (NaviChar)
	{
		NaviChar->SetIsPlayer(true);
		NaviChar->GetStatComponent()->InitPlayerStat();
		InitInventory();
	}
}

void ANaviPlayerController::PickupItem(AItem* Item)
{
	if (Item == nullptr) { return; }

	Inventory[ItemIDToIndex(Item->GetItemID())] = Item;
	Item->SetItemState(EItemState::EIS_PickedUp);

	OnInventoryItemUpdateDelegate.Broadcast();
	UploadInventory();
}


void ANaviPlayerController::InitInventory()
{
	auto NaviGameInstance = Cast<UNaviGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	for (int Index = 0; Index < INVENTORY_CAPACITY; ++Index)
	{
		EInventoryItemState StoredItemState = NaviGameInstance->NaviInventory[Index];

		if ((StoredItemState == EInventoryItemState::EIIS_Have) || 
			(StoredItemState == EInventoryItemState::EIIS_Equipped))
		{
			if (Inventory[Index]->IsValidLowLevel() == false)
			{
				Inventory[Index] = SpawnItemFromTable(IndexToItemID(Index));
				Inventory[Index]->SetItemState(EItemState::EIS_PickedUp);
			}
			if ((StoredItemState == EInventoryItemState::EIIS_Equipped))
			{
				Cast<ANaviCharacter>(GetPawn())->EquipItem(Inventory[Index]);
			}
		}
	}

	OnInventoryItemUpdateDelegate.Broadcast();
}

AItem* ANaviPlayerController::SpawnItemFromTable(FName ItemID)
{
	if (BPTableObject)
	{
		FItemBPTable* ItemBPRow = nullptr;
		ItemBPRow = BPTableObject->FindRow<FItemBPTable>(ItemID, TEXT(""));

		if (ItemBPRow)
		{
			return GetWorld()->SpawnActor<AItem>(ItemBPRow->ItemClass);
		}
	}
	return nullptr;
}

void ANaviPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("EscapeButton", IE_Pressed, this, &ANaviPlayerController::EscapeButtonPressed);
	InputComponent->BindAction("InventoryButton", IE_Pressed, this, &ANaviPlayerController::InventoryButtonPressed);
}

void ANaviPlayerController::EscapeButtonPressed()
{
	OnEscapeWindowDelegate.Broadcast();
}

void ANaviPlayerController::InventoryButtonPressed()
{
	OnInventoryWindowDelegate.Broadcast();
}

void ANaviPlayerController::SwapItem(int32 Index)
{
	auto NaviChar = Cast<ANaviCharacter>(GetPawn());
	if (NaviChar)
	{
		NaviChar->SwapItem(Inventory[Index]);

		OnItemEquipStateUpdateDelegate.Broadcast();
		UploadInventory();
	}
}

FName ANaviPlayerController::IndexToItemID(int Index)
{
	FName ItemID;
	if (Index == 0)
	{
		ItemID = TEXT("W_SMG_C_1");
	}
	if (Index == 1)
	{
		ItemID = TEXT("W_SMG_R_1");
	}
	if (Index == 2)
	{
		ItemID = TEXT("W_SMG_L_1");
	}
	if (Index == 3)
	{
		ItemID = TEXT("W_AR_C_1");
	}
	if (Index == 4)
	{
		ItemID = TEXT("W_AR_R_1");
	}
	if (Index == 5)
	{
		ItemID = TEXT("W_AR_L_1");
	}
	if (Index == 6)
	{
		ItemID = TEXT("W_PIS_C_1");
	}
	if (Index == 7)
	{
		ItemID = TEXT("W_PIS_R_1");
	}
	if (Index == 8)
	{
		ItemID = TEXT("W_PIS_L_1");
	}
	if (Index == 9)
	{
		ItemID = TEXT("A_RING_R_1");
	}
	if (Index == 10)
	{
		ItemID = TEXT("A_RING_L_1");
	}
	if (Index == 11)
	{
		ItemID = TEXT("A_NECK_R_1");
	}
	if (Index == 12)
	{
		ItemID = TEXT("A_NECK_L_1");
	}
	return ItemID;
}

int ANaviPlayerController::ItemIDToIndex(FName ItemID)
{
	if (ItemID == TEXT("W_SMG_C_1"))
	{
		return 0;
	}
	if (ItemID == TEXT("W_SMG_R_1"))
	{
		return 1;
	}
	if (ItemID == TEXT("W_SMG_L_1"))
	{
		return 2;
	}

	if (ItemID == TEXT("W_AR_C_1"))
	{
		return 3;
	}
	if (ItemID == TEXT("W_AR_R_1"))
	{
		return 4;
	}
	if (ItemID == TEXT("W_AR_L_1"))
	{
		return 5;
	}

	if (ItemID == TEXT("W_PIS_C_1"))
	{
		return 6;
	}
	if (ItemID == TEXT("W_PIS_R_1"))
	{
		return 7;
	}
	if (ItemID == TEXT("W_PIS_L_1"))
	{
		return 8;
	}

	if (ItemID == TEXT("A_RING_R_1"))
	{
		return 9;
	}
	if (ItemID == TEXT("A_RING_L_1"))
	{
		return 10;
	}
	if (ItemID == TEXT("A_NECK_R_1"))
	{
		return 11;
	}
	if (ItemID == TEXT("A_NECK_L_1"))
	{
		return 12;
	}
	return -1;
}

void ANaviPlayerController::UploadInventory()
{
	auto NaviGameInstance = Cast<UNaviGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	for (int Index = 0; Index < INVENTORY_CAPACITY; ++Index)
	{
		if (Inventory[Index]->IsValidLowLevel())
		{
			NaviGameInstance->NaviInventory[Index] = EInventoryItemState::EIIS_Have;
		}
	}

	auto NaviChar = Cast<ANaviCharacter>(GetPawn());

	int EquipWeaponIndex = ItemIDToIndex(NaviChar->GetEquippedWeapon()->GetItemID());
	NaviGameInstance->NaviInventory[EquipWeaponIndex] = EInventoryItemState::EIIS_Equipped;

	if (NaviChar->GetEquippedAccessory())
	{
		int EquipAccIndex = ItemIDToIndex(NaviChar->GetEquippedAccessory()->GetItemID());
		NaviGameInstance->NaviInventory[EquipAccIndex] = EInventoryItemState::EIIS_Equipped;
	}

	NaviGameInstance->UploadInventory();
}