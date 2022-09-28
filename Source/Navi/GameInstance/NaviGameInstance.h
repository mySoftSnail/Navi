// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Navi/Components/NaviStatComponent.h"
#include "Navi/Controllers/NaviPlayerController.h"
#include "NaviGameInstance.generated.h"

#define USESERVER if (UseServer == false) { return; } 

UENUM(BlueprintType)
enum class EServerConnectionResultType : uint8
{
	ESCRT_ConnectionFailure UMETA(DisplayName = "ConnectionFailure"),
	ESCRT_ConnectionSuccess UMETA(DisplayName = "ConnectionSuccess"),
};

UENUM(BlueprintType)
enum class ELoginResultType : uint8
{
	ELRT_ServerError UMETA(DisplayName = "ServerError"),
	ELRT_LoginSuccess UMETA(DisplayName = "LoginSuccess"),
	ELRT_NoExistID UMETA(DisplayName = "NoExistID"),
	ELRT_WrongPassword UMETA(DisplayName = "WrongPassword"),
};

UENUM(BlueprintType)
enum class ESignUpResultType : uint8
{
	ESRT_ServerError UMETA(DisplayName = "ServerError"),
	ESRT_IDConflict UMETA(DisplayName = "IDConflict"),
	ESRT_SignUpSuccess UMETA(DisplayName = "SignUpSuccess"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateServerConnectionResultDelegate, EServerConnectionResultType, ServerConnectionResultType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateLoginResultDelegate, ELoginResultType, LoginResultType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateSignUpResultDelegate, ESignUpResultType, SignUpResultType);

/**
 * 
 */
UCLASS()
class NAVI_API UNaviGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UNaviGameInstance(const class FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
	void NaviInit();

protected:
	virtual void Init() override;

	void RequestHttp(const FString& URL, const FString& Verb, void (UNaviGameInstance::* InFunc)(FHttpRequestPtr, FHttpResponsePtr, bool), const FString ContentString = "");

	TSharedPtr<FJsonObject> GetResponseJsonObject(FHttpResponsePtr Response);

	/** 서버 연결 상태 확인 콜백 함수 */
	void OnConnectionTestRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 회원가입 요청을 서버로 보내는 함수 */
	UFUNCTION(BlueprintCallable)
	void PostSignup(FString IDString, FString PWString);

	/** 회원가입 요청 콜백 함수 */
	void OnSignupResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 로그인 요청을 서버로 보내는 함수 */
	UFUNCTION(BlueprintCallable)
	void PostLogin(FString IDString, FString PWString);

	/** 로그인 요청 콜백 함수 */
	void OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 새로운 캐릭터 생성 요청을 서버로 보내는 함수 */
	void CreateCharacter();

	/** 캐릭터 생성 요청 콜백 함수 */
	void OnCreateCharacterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 캐릭터 정보 조회 요청을 서버로 보내는 함수 */
	void GetCharacterInfo();

	/** 캐릭터 정보 조회 요청 콜백 함수 */
	void OnGetCharacterInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 캐릭터 스탯 정보 업로드 콜백 함수 */
	void OnUploadStatResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 인벤토리 정보 조회 요청을 서버로 보내는 함수 */
	void GetInventoryInfo();

	/** 인벤토리 정보 조회 요청 콜백 함수 */
	void OnGetInventoryInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** 인벤토리 아이템 추가 요청 콜백 함수 */
	void OnUploadInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	/** 받은 문자열에 특수문자가 들었는지 확인하는 함수 */
	UFUNCTION(BlueprintCallable)
	bool IsContainSpecialSymbol(FString InputString);

	/** 캐릭터 스탯 정보를 서버로 업로드하는 함수 */
	void UploadStat();

	/** 캐릭터 인벤토리 정보를 서버로 업로드하는 함수 */
	void UploadInventory();

	EInventoryItemState StringToEnum(FString InString);

private:
	// 개발 중 서버 기능 사용 여부
	bool UseServer = true;

	static const FString Address;

	FHttpModule* HttpModule;

	// 회원가입, 로그인 ID 저장 변수
	FString SignupID;
	FString LoginID;

	// 게임 중 유저 ID
	FString UserID;

public:
	/** 서버 연결 결과 창 UI 바인딩 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = NaviServer)
	FOnUpdateServerConnectionResultDelegate OnUpdateServerConnectionResultDelegate;

	/** 로그인 결과 창 UI 바인딩 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = NaviServer)
	FOnUpdateLoginResultDelegate OnUpdateLoginResultDelegate;

	/** 회원가입 결과 창 UI 바인딩 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = NaviServer)
	FOnUpdateSignUpResultDelegate OnUpdateSignUpResultDelegate;

	/** 캐릭터 스탯 저장 구조체 */
	FNaviStat NaviStat;

	/** 캐릭터 인벤토리 상태 저장 배열 */
	EInventoryItemState NaviInventory[13];

};
