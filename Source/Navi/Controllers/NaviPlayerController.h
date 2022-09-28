// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DataTable.h"
#include "NaviPlayerController.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemState : uint8
{
	EIIS_NoHave UMETA(DisplayName = "NoHave"),
	EIIS_Have UMETA(DisplayName = "Have"),
	EIIS_Equipped UMETA(DisplayName = "Equipped"),
};

class AItem;
class AWeapon;
class UUserWidget;
class ANaviGameModeBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCommandTabOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCommandTabOff);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEscapeWindowDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryWindowDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryItemUpdateDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemEquipStateUpdateDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDieDelegate);

UCLASS()
class NAVI_API ANaviPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ANaviPlayerController();

	void PickupItem(AItem* Item);

	void UploadInventory();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void InitInventory();

	AItem* SpawnItemFromTable(FName ItemID);

	virtual void SetupInputComponent() override;

protected:
	void EscapeButtonPressed();

	void InventoryButtonPressed();

	UFUNCTION(BlueprintCallable)
	void SwapItem(int32 Index);

	FName IndexToItemID(int Index);

	UFUNCTION(BlueprintCallable)
	int ItemIDToIndex(FName ItemID);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 13 };

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HUDOverlayClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Widgets)
	FOnEscapeWindowDelegate OnEscapeWindowDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Widgets)
	FOnInventoryWindowDelegate OnInventoryWindowDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Widgets)
	FOnInventoryItemUpdateDelegate OnInventoryItemUpdateDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Widgets)
	FOnItemEquipStateUpdateDelegate OnItemEquipStateUpdateDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* BPTableObject;

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Widgets)
	FOnPlayerDieDelegate OnPlayerDieDelegate;
	
};
