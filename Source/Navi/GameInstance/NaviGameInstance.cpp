// Fill out your copyright notice in the Description page of Project Settings.


#include "NaviGameInstance.h"

const FString UNaviGameInstance::Address(TEXT("http://14.34.64.174:3000"));

UNaviGameInstance::UNaviGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HttpModule = &FHttpModule::Get();
}

void UNaviGameInstance::Init()
{
	Super::Init();

	NaviInit();
}

void UNaviGameInstance::RequestHttp(const FString& URL, const FString& Verb, void(UNaviGameInstance::* InFunc)(FHttpRequestPtr, FHttpResponsePtr, bool), const FString ContentString)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();
	FString URLString = URL;
	HttpRequest->OnProcessRequestComplete().BindUObject(this, InFunc);
	HttpRequest->SetVerb(Verb);
	if (ContentString != "") 
	{
		if (Verb == "GET")
		{
			URLString += ContentString;
		}
		else
		{
			HttpRequest->SetContentAsString(ContentString);
		}
		HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	}
	else
	{
		HttpRequest->SetHeader("Content-Type", "application/json");
	}
	HttpRequest->SetURL(URLString);
	HttpRequest->SetHeader("User-Agent", "X-UnrealEngine-Agent");
	HttpRequest->ProcessRequest();
}

TSharedPtr<FJsonObject> UNaviGameInstance::GetResponseJsonObject(FHttpResponsePtr Response)
{
	FString JsonString = Response->GetContentAsString();

	// Json Data를 읽기 위한 Reader 포인터 생성
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonString);

	// 직렬화 된 Json 데이터를 hold 하기 위한 포인터 생성
	TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject());

	FJsonSerializer::Deserialize(Reader, ResponseObj);

	return ResponseObj;
}

void UNaviGameInstance::NaviInit()
{
	NaviStat.Level = 1;
	NaviStat.EXP = 0.f;
	NaviStat.SkillOneLevel = 1;
	NaviStat.SkillTwoLevel = 1;
	NaviStat.SkillPoint = 0;

	for (auto Item : NaviInventory)
	{
		Item = EInventoryItemState::EIIS_NoHave;
	}
	NaviInventory[0] = EInventoryItemState::EIIS_Equipped; // 게임 시작 시 기본 무기 Common SMG 착용

	USESERVER;

	RequestHttp(Address + "/connection-test", "GET", &UNaviGameInstance::OnConnectionTestRequestReceived);
}

void UNaviGameInstance::OnConnectionTestRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	if (bWasSuccessful)
	{
		OnUpdateServerConnectionResultDelegate.Broadcast(EServerConnectionResultType::ESCRT_ConnectionSuccess);
	}
	else
	{
		OnUpdateServerConnectionResultDelegate.Broadcast(EServerConnectionResultType::ESCRT_ConnectionFailure);
	}
}

void UNaviGameInstance::PostSignup(FString IDString, FString PWString)
{
	USESERVER;

	SignupID = IDString; // 입력 저장
	const FString RequestString = TEXT("signup_id=") + IDString + TEXT("&signup_pw=") + PWString;
	RequestHttp(Address + "/user/signup", "POST", &UNaviGameInstance::OnSignupResponseReceived, RequestString);
}

void UNaviGameInstance::OnSignupResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);

	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	if (ResponseCode == 200)
	{
		OnUpdateSignUpResultDelegate.Broadcast(ESignUpResultType::ESRT_SignUpSuccess);
		CreateCharacter();
	}
	else if (ResponseCode == 409)
	{
		OnUpdateSignUpResultDelegate.Broadcast(ESignUpResultType::ESRT_IDConflict);
	}
	else if(ResponseCode == 500)
	{
		OnUpdateSignUpResultDelegate.Broadcast(ESignUpResultType::ESRT_ServerError);
	}
	else
	{
		OnUpdateSignUpResultDelegate.Broadcast(ESignUpResultType::ESRT_ServerError);
	}
}

void UNaviGameInstance::PostLogin(FString IDString, FString PWString)
{
	USESERVER;

	LoginID = IDString; // 입력 저장
	const FString RequestString = TEXT("login_id=") + IDString + TEXT("&login_pw=") + PWString;
	RequestHttp(Address + "/user/login", "POST", &UNaviGameInstance::OnLoginResponseReceived, RequestString);
}

void UNaviGameInstance::OnLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	if (ResponseCode == 200)
	{
		// 게임을 시작할 유저 ID 저장
		UserID = LoginID;

		// 로그인 성공시 캐릭터 정보 불러오기
		GetCharacterInfo();
		GetInventoryInfo();

		OnUpdateLoginResultDelegate.Broadcast(ELoginResultType::ELRT_LoginSuccess);
	}
	else if (ResponseCode == 401)
	{
		OnUpdateLoginResultDelegate.Broadcast(ELoginResultType::ELRT_WrongPassword);
	}
	else if (ResponseCode == 404)
	{
		OnUpdateLoginResultDelegate.Broadcast(ELoginResultType::ELRT_NoExistID);
	}
	else if (ResponseCode == 500)
	{
		OnUpdateLoginResultDelegate.Broadcast(ELoginResultType::ELRT_ServerError);
	}
	else
	{
		OnUpdateLoginResultDelegate.Broadcast(ELoginResultType::ELRT_ServerError);
	}
}

void UNaviGameInstance::CreateCharacter()
{
	USESERVER;

	const FString RequestString = TEXT("character_id=Player_") + SignupID + TEXT("&user_id=") + SignupID;
	RequestHttp(Address + "/character", "POST", &UNaviGameInstance::OnCreateCharacterResponseReceived, RequestString);
}

void UNaviGameInstance::OnCreateCharacterResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	check(ResponseCode == 200);
}

void UNaviGameInstance::GetCharacterInfo()
{
	USESERVER;

	const FString QueryString = TEXT("?user_id=") + UserID;
	RequestHttp(Address + "/character", "GET", &UNaviGameInstance::OnGetCharacterInfoResponseReceived, QueryString);
}

void UNaviGameInstance::OnGetCharacterInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	check(ResponseCode == 200);

	NaviStat.Level = ResponseObj->GetIntegerField("level");
	NaviStat.SkillOneLevel = ResponseObj->GetIntegerField("skill_one_level");
	NaviStat.SkillTwoLevel = ResponseObj->GetIntegerField("skill_two_level");
	NaviStat.EXP = (float)ResponseObj->GetNumberField("exp");
	NaviStat.SkillPoint = (float)ResponseObj->GetNumberField("skill_point");
}

void UNaviGameInstance::UploadStat()
{
	USESERVER;

	const FString RequestString =
		TEXT("character_id=Player_") + UserID +
		TEXT("&level=") + FString::FromInt(NaviStat.Level) +
		TEXT("&exp=") + FString::FromInt(NaviStat.EXP) +
		TEXT("&skill_one_level=") + FString::FromInt(NaviStat.SkillOneLevel) +
		TEXT("&skill_two_level=") + FString::FromInt(NaviStat.SkillTwoLevel) +
		TEXT("&skill_point=") + FString::FromInt(NaviStat.SkillPoint);

	RequestHttp(Address + "/character", "PUT", &UNaviGameInstance::OnUploadStatResponseReceived, RequestString);
}

void UNaviGameInstance::OnUploadStatResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	check(ResponseCode == 200);
}

void UNaviGameInstance::UploadInventory()
{
	USESERVER;

	const FString RequestString =
		TEXT("character_id=Player_") + UserID +
		TEXT("&smg_common=") + FString::FromInt((int)NaviInventory[0]) +
		TEXT("&smg_rare=") + FString::FromInt((int)NaviInventory[1]) +
		TEXT("&smg_legendary=") + FString::FromInt((int)NaviInventory[2]) +
		TEXT("&ar_common=") + FString::FromInt((int)NaviInventory[3]) +
		TEXT("&ar_rare=") + FString::FromInt((int)NaviInventory[4]) +
		TEXT("&ar_legendary=") + FString::FromInt((int)NaviInventory[5]) +
		TEXT("&pis_common=") + FString::FromInt((int)NaviInventory[6]) +
		TEXT("&pis_rare=") + FString::FromInt((int)NaviInventory[7]) +
		TEXT("&pis_legendary=") + FString::FromInt((int)NaviInventory[8]) +
		TEXT("&ring_rare=") + FString::FromInt((int)NaviInventory[9]) +
		TEXT("&ring_legendary=") + FString::FromInt((int)NaviInventory[10]) +
		TEXT("&necklace_rare=") + FString::FromInt((int)NaviInventory[11]) +
		TEXT("&necklace_legendary=") + FString::FromInt((int)NaviInventory[12]);

	RequestHttp(Address + "/inventory", "PUT", &UNaviGameInstance::OnUploadInventoryResponseReceived, RequestString);
}

void UNaviGameInstance::OnUploadInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	check(ResponseCode == 200);
}

void UNaviGameInstance::GetInventoryInfo()
{
	USESERVER;

	const FString QueryString = TEXT("?character_id=Player_") + UserID;
	RequestHttp(Address + "/inventory", "GET", &UNaviGameInstance::OnGetInventoryInfoResponseReceived, QueryString);
}

void UNaviGameInstance::OnGetInventoryInfoResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	USESERVER;

	TSharedPtr<FJsonObject> ResponseObj = GetResponseJsonObject(Response);
	int32 ResponseCode = ResponseObj->GetIntegerField("code");

	check(ResponseCode == 200);

	NaviInventory[0] = StringToEnum(*ResponseObj->GetStringField("smg_common"));
	NaviInventory[1] = StringToEnum(*ResponseObj->GetStringField("smg_rare"));
	NaviInventory[2] = StringToEnum(*ResponseObj->GetStringField("smg_legendary"));
	NaviInventory[3] = StringToEnum(*ResponseObj->GetStringField("ar_common"));
	NaviInventory[4] = StringToEnum(*ResponseObj->GetStringField("ar_rare"));
	NaviInventory[5] = StringToEnum(*ResponseObj->GetStringField("ar_legendary"));
	NaviInventory[6] = StringToEnum(*ResponseObj->GetStringField("pis_common"));
	NaviInventory[7] = StringToEnum(*ResponseObj->GetStringField("pis_rare"));
	NaviInventory[8] = StringToEnum(*ResponseObj->GetStringField("pis_legendary"));
	NaviInventory[9] = StringToEnum(*ResponseObj->GetStringField("ring_rare"));
	NaviInventory[10] = StringToEnum(*ResponseObj->GetStringField("ring_legendary"));
	NaviInventory[11] = StringToEnum(*ResponseObj->GetStringField("necklace_rare"));
	NaviInventory[12] = StringToEnum(*ResponseObj->GetStringField("necklace_legendary"));
}

bool UNaviGameInstance::IsContainSpecialSymbol(FString InputString)
{
	for (int Index = 0; Index < InputString.Len(); ++Index)
	{
		int32 AsciiCode = (int32)InputString[Index];

		if (AsciiCode >= 48 && AsciiCode <= 57)	// 아스키코드로 숫자 0~9는 10진수 48~57
		{
			continue;
		}
		else if (AsciiCode >= 65 && AsciiCode <= 90) // 아스키코드로 영문 대문자 A~Z는 10진수 65~90
		{
			continue;
		}
		else if (AsciiCode >= 97 && AsciiCode <= 122) // 아스키코드로 영문 소문자 a~z는 10진수 97~122
		{
			continue;
		}
		else
		{
			return true; // 나머지 문자는 특수문자로 판단하고 true 리턴
		}
	}

	// 모든 문자 검사 완료 결과 특수문자 포함되지 않음.
	return false;
}

EInventoryItemState UNaviGameInstance::StringToEnum(FString InString)
{
	if (InString == TEXT("NoHave"))
	{
		return EInventoryItemState::EIIS_NoHave;
	}
	else if (InString == TEXT("Have"))
	{
		return EInventoryItemState::EIIS_Have;
	}
	else if (InString == TEXT("Equipped"))
	{
		return EInventoryItemState::EIIS_Equipped;
	}
	return EInventoryItemState::EIIS_NoHave;
}