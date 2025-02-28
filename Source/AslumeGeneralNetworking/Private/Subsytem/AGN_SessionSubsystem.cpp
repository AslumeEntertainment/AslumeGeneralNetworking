// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#include "Subsystems/AGN_SessionSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"


DEFINE_LOG_CATEGORY_STATIC(AslumeGeneralNetwoking_SessionSubsystem, Log, All);

bool UAGN_SessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer))
	{
		return true;
	}
	return false;
}

void UAGN_SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();

	if (IsValid(World) == false)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Initialize IsValid(World) == false"));
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(World);
	if (OnlineSubsystem == nullptr)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Initialize IsValid(OnlineSubsystem) == false"));
		return;
	}

	SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (SessionInterface == nullptr)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Initialize IsValid(SessionInterface) == false"));
	}

	SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UAGN_SessionSubsystem::OnSessionUserInviteAccepted);

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleCreateSessionComplete);
	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleDestroySessionComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleJoinSessionComplete);

	LocalSessionState = EAGN_HostingState::Idle;
}

void UAGN_SessionSubsystem::Deinitialize()
{
	SessionInterface->ClearOnSessionUserInviteAcceptedDelegates(this);
	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
	SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
	SessionInterface = nullptr;
}

void UAGN_SessionSubsystem::RegisterExistingPlayers()
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("RegisterExistingPlayers IsValid(World) == false"));
		return;
	}

	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		RegisterNewPlayer(PlayerController);
	}
}

void UAGN_SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UAGN_SessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UAGN_SessionSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int ControllerId, TSharedPtr<const FUniqueNetId, ESPMode::ThreadSafe> UserId, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
}

void UAGN_SessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
}

bool UAGN_SessionSubsystem::DefaultCreateSession()
{
	return CreateSession(0, GameSessionLocalName, NumPublicConnections, NumPrivateConnections, bIsLANMatch, bShouldAdvertise, bUsesPresence, bUseLobbiesIfAvailable, bAllowInvites, bAllowJoinViaPresence, bAllowJoinInProgress);
}

bool UAGN_SessionSubsystem::CreateSession(int32 HostingPlayerNum, FName SessionName, int InNumPublicConnections, int InNumPrivateConnections, bool InbIsLANMatch, bool InbShouldAdvertise, bool InbUsesPresence, bool InbUseLobbiesIfAvailable, bool InbAllowInvites, bool InbAllowJoinViaPresence, bool InbAllowJoinInProgress)
{
	return false;
}


bool UAGN_SessionSubsystem::JoinSession(int32 PlayerNum, FName SessionName)
{
}

bool UAGN_SessionSubsystem::DestroySession(bool InbShouldStayIdle)
{
}

void UAGN_SessionSubsystem::RegisterNewPlayer(APlayerController* InPlayerController)
{
}

void UAGN_SessionSubsystem::UnRegisterPlayer(APlayerController* InPlayerController)
{
}

void UAGN_SessionSubsystem::InviteFriendToSession(FUniqueNetIdRef FriendNetIdPtr)
{
}
