// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#include "Subsystems/AGN_IdentitySubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

DEFINE_LOG_CATEGORY_STATIC(AslumeGeneralNetwoking_IdentitySubsystem, Log, All);

//---------------------------------------------------------------INITIALIZATION------------------------------------------

bool UAGN_IdentitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer))
	{
		return true;
	}
	return false;
}

void UAGN_IdentitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();

	if (IsValid(World) == false)
	{
		UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Error, TEXT("Initialize IsValid(World) == false"));
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World);
	if (OnlineSubsystem == nullptr)
	{
		UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Error, TEXT("Initialize IsValid(OnlineSubsystem) == false"));
		return;
	}

	IdentityInterface = OnlineSubsystem->GetIdentityInterface();

	if (IdentityInterface == nullptr)
	{
		UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Error, TEXT("Initialize IsValid(IdentityInterface) == false"));
	}
}

void UAGN_IdentitySubsystem::Deinitialize()
{
	IdentityInterface = nullptr;
}
//---------------------------------------------------------------INITIALIZATION------------------------------------------



//---------------------------------------------------------------LOGIN-------------------------------------------------

bool UAGN_IdentitySubsystem::Login(FString Id, FString Token, FString Type)
{

	IdentityInterface->OnLoginCompleteDelegates->AddUObject(this, &UAGN_IdentitySubsystem::HandleLoginComplete);

	FOnlineAccountCredentials Credentials;
	Credentials.Id = Id;
	Credentials.Token = Token;
	Credentials.Type = Type;

	if (!IdentityInterface->Login(0, Credentials))
	{
		UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Error, TEXT("Could not start login for local user..."));
		return false;
	}

	return true;
}

void UAGN_IdentitySubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IdentityInterface->ClearOnLoginCompleteDelegates(0, this);

	OnLoginCompleted(LocalUserNum, bWasSuccessful, GetLocalPlayerDisplayName(), Error);
}

void UAGN_IdentitySubsystem::OnLoginCompleted_Implementation(int32 LocalUserNum, bool bWasSuccessful, const FString& PlayerName, const FString& Error)
{
	if (!bWasSuccessful)
	{
		UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Warning, TEXT("Cloud NOT login player[%s] Error: %s"), *PlayerName, *Error);
		return;
	}
	UE_LOG(AslumeGeneralNetwoking_IdentitySubsystem, Display, TEXT("Login player[%s] completed!"), *PlayerName);
}
//---------------------------------------------------------------LOGIN-------------------------------------------------



//---------------------------------------------------------------UTILITY-------------------------------------------------

FUniqueNetIdPtr UAGN_IdentitySubsystem::GetLocalPlayerNetId()
{
	return IdentityInterface->GetUniquePlayerId(0);
}

FString UAGN_IdentitySubsystem::GetLocalPlayerDisplayName()
{
	return IdentityInterface->GetPlayerNickname(*GetLocalPlayerNetId());
}

bool UAGN_IdentitySubsystem::IsPlayerLoggedIn()
{
	return GetLocalPlayerNetId() != nullptr;
}
//---------------------------------------------------------------UTILITY-------------------------------------------------