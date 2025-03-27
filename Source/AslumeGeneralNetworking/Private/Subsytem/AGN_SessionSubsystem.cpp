// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#include "Subsystems/AGN_SessionSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Subsystems/AGN_IdentitySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(AslumeGeneralNetwoking_SessionSubsystem, Log, All);

const FName UAGN_SessionSubsystem::GameIDSetting = FName("GameID");
const FName UAGN_SessionSubsystem::SessionNameSetting = FName("SessionName");

TArray<FOnlineSessionSearchResultWrapper> UAGN_SessionSubsystem::WrapSearchResults(TArray<FOnlineSessionSearchResult> FoundSessions)
{
	TArray<FOnlineSessionSearchResultWrapper> FoundSessionsWrapped = TArray<FOnlineSessionSearchResultWrapper>();

	for (FOnlineSessionSearchResult Session : FoundSessions)
	{
		FOnlineSessionSearchResultWrapper SessionWrapped = FOnlineSessionSearchResultWrapper();
		if (!Session.Session.SessionSettings.Get<FString>(SessionNameSetting, SessionWrapped.SessionName))
		{
			UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Warning, TEXT("UAGN_SessionSubsystem::WrapSearchResults - Problem Getting Session Name"));
		}
		SessionWrapped.Session = Session.Session;
		SessionWrapped.PingInMs = Session.PingInMs;
		FoundSessionsWrapped.Add(SessionWrapped);
	}

	return FoundSessionsWrapped;
}


//---------------------------------------------------------------INITIALIZATION------------------------------------------

bool UAGN_SessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
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

	SessionInterface->OnSessionParticipantJoinedDelegates.AddUObject(this, &UAGN_SessionSubsystem::RegisterNewPlayer);
	SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(this, &UAGN_SessionSubsystem::UnRegisterPlayer);

	LocalHostingState = EAGN_HostingState::Idle;
}

void UAGN_SessionSubsystem::Deinitialize()
{

	SessionInterface->ClearOnSessionUserInviteAcceptedDelegates(this);

	SessionInterface->ClearOnSessionParticipantJoinedDelegates(this);
	SessionInterface->ClearOnSessionParticipantLeftDelegates(this);

	SessionInterface = nullptr;
}
//---------------------------------------------------------------INITIALIZATION------------------------------------------



//---------------------------------------------------------------REGISTERING WORK------------------------------------------

void UAGN_SessionSubsystem::RegisterNewPlayer(FName SessionName, const FUniqueNetId& PlayerID)
{
	if (SessionInterface->IsPlayerInSession(GameSessionLocalName, PlayerID))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("New Player[%s] alredy registered in session: %s"), *PlayerID.ToString(), *SessionName.ToString());
		return;
	}

	if (SessionInterface->RegisterPlayer(SessionName, PlayerID, false))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("New Player[%s] being Registered in session: %s..."), *PlayerID.ToString(), *SessionName.ToString());
		return;
	}

	// The player could not be registered; typically you will want to kick the player from the server in this situation.
	UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Warning, TEXT("New Player[%s] could NOT be registered in session: %s. Removing the player..."), *PlayerID.ToString(), *SessionName.ToString());
	SessionInterface->RemovePlayerFromSession(0, SessionName, PlayerID);

}

void UAGN_SessionSubsystem::HandleRegisterNewPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		for (FUniqueNetIdRef PlayerID : Players)
		{
			UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Player[%s] could NOT be Registered in session: %s"), *PlayerID.Get().ToString(), *SessionName.ToString());
			SessionInterface->RemovePlayerFromSession(0, SessionName, PlayerID.Get());
		}
		
		return;
	}

	for (FUniqueNetIdRef PlayerID : Players)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Player[%s] Registered Successfully in session: %s"), *PlayerID.Get().ToString(), *SessionName.ToString());
	}
}

void UAGN_SessionSubsystem::UnRegisterPlayer(FName SessionName, const FUniqueNetId& PlayerID, EOnSessionParticipantLeftReason Reason)
{
	if (!SessionInterface->IsPlayerInSession(GameSessionLocalName, PlayerID))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Player[%s] is NOT in session: %s"), *PlayerID.ToString(), *SessionName.ToString());
		return;
	}

	if (!SessionInterface->UnregisterPlayer(SessionName, PlayerID))
	{
		
		return;
	}
	UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Player[%s] leaving session: %s..."), *PlayerID.ToString(), *SessionName.ToString());
}

void UAGN_SessionSubsystem::HandleUnRegisterPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		for (FUniqueNetIdRef PlayerID : Players)
		{
			UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Player[%s] could NOT be UnRegistered from session: %s"), *PlayerID.Get().ToString(), *SessionName.ToString());
		}
		return;
	}

	for (FUniqueNetIdRef PlayerID : Players)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Player[%s] UnRegistered Successfully from session: %s"), *PlayerID.Get().ToString(), *SessionName.ToString());
	}
}
//---------------------------------------------------------------REGISTERING WORK------------------------------------------



//---------------------------------------------------------------CREATING SESSION------------------------------------------

bool UAGN_SessionSubsystem::DefaultCreateSession(FString SessionName)
{
	return CreateSession(0, SessionName, DefaultSessionSettings);
}

bool UAGN_SessionSubsystem::CreateSession(int32 HostingPlayerNum, FString SessionName, FSessionSettingsWrapper SessionSettings)
{
	FOnlineSessionSettings SessionSettingsRef = SessionSettings.Get();

	// You *must* set at least one setting value, because you can not run FindSessions without any filters.

	//SessionSettingsRef
	SessionSettingsRef.Settings.Add(
		GameIDSetting,
		FOnlineSessionSetting(GameID, EOnlineDataAdvertisementType::ViaOnlineService));

	SessionSettingsRef.Settings.Add(
		SessionNameSetting,
		FOnlineSessionSetting(SessionName, EOnlineDataAdvertisementType::ViaOnlineService));

	

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleCreateSessionComplete);

	if (!SessionInterface->CreateSession(0, GameSessionLocalName, SessionSettingsRef))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Creation of session: %s cloud not be started..."), *SessionName);

		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

		return false;
	}

	return true;
}

void UAGN_SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

	if (bWasSuccessful == false)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Session: %s cloud NOT be created"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Created Session"));
		LocalHostingState = EAGN_HostingState::Hosting;
	}

	OnCreateSessionCompleted.Broadcast(SessionName, bWasSuccessful);
}
//---------------------------------------------------------------CREATING SESSION------------------------------------------



//---------------------------------------------------------------FINDING SESSION------------------------------------------

bool UAGN_SessionSubsystem::DefautFindSessions()
{
	return FindSessions(DefaultSearchSettings);
}

bool UAGN_SessionSubsystem::FindSessions(FSessionSearchWrapper SearchSettings)
{
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleFindSessionsComplete);

	CurrentSearchSettings = SearchSettings.Get();

	CurrentSearchSettings->QuerySettings.SearchParams.Add(
		GameIDSetting,
		FOnlineSessionSearchParam(GameID));

	CurrentSearchSettings->QuerySettings.Set(FName(TEXT("LOBBYSEARCH")), true, EOnlineComparisonOp::Equals);

	if (!SessionInterface->FindSessions(0, CurrentSearchSettings.ToSharedRef()))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Searching for sessions cloud not be started..."));

		SessionInterface->ClearOnFindSessionsCompleteDelegates(this);

		CurrentSearchSettings = MakeShareable(new FOnlineSessionSearch());

		return false;
	}

	return true;
}

void UAGN_SessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	SessionInterface->ClearOnFindSessionsCompleteDelegates(this);

	if (bWasSuccessful == false)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Session Search failed"));
		CurrentSearchSettings = MakeShareable(new FOnlineSessionSearch());
	}
	else
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Found some sessions"));
	}

	OnFindSessionsCompleted.Broadcast(UAGN_SessionSubsystem::WrapSearchResults(CurrentSearchSettings->SearchResults), bWasSuccessful);
}
//---------------------------------------------------------------FINDING SESSION------------------------------------------



//---------------------------------------------------------------JOINING SESSION------------------------------------------

bool UAGN_SessionSubsystem::JoinSession(int32 PlayerNum, FOnlineSessionSearchResultWrapper Session)
{
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleJoinSessionComplete);

	FOnlineSessionSearchResult SessiontoJoin = Session.Get();

	SessionInterface->GetResolvedConnectString(SessiontoJoin, NAME_GamePort, CurrentJoiningSessionConnectInfo);

	if (!SessionInterface->JoinSession(0, GameSessionLocalName, SessiontoJoin))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Joining session %s not be started..."), *Session.SessionName);
		return false;
	}

	return true;
}

void UAGN_SessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);

	bool bIsIn = (JoinResult == EOnJoinSessionCompleteResult::Success || JoinResult == EOnJoinSessionCompleteResult::AlreadyInSession);

	if (!bIsIn)
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Warning, TEXT("Could NOT join session"));
	}
	else
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Session Joined"));
		LocalHostingState = EAGN_HostingState::Joined;

		UWorld* World = GetWorld();
		if (IsValid(World) == false)
		{
			UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("UAGN_SessionSubsystem::HandleJoinSessionComplete IsValid(World) == false"));
			return;
		}
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (IsValid(PC) == false)
		{
			UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("UAGN_SessionSubsystem::HandleJoinSessionComplete IsValid(PC) == false"));
			return;
		}
		PC->ClientTravel(CurrentJoiningSessionConnectInfo, TRAVEL_Absolute);

	}

	CurrentJoiningSessionConnectInfo = "";
	OnJoinSessionCompleted.Broadcast(bIsIn);
}
//---------------------------------------------------------------JOINING SESSION------------------------------------------



//---------------------------------------------------------------DESTROYING SESSION------------------------------------------

bool UAGN_SessionSubsystem::DestroySession(bool InbShouldStayIdle)
{

	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleDestroySessionComplete);
	return false;
}

void UAGN_SessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
}
//---------------------------------------------------------------DESTROYING SESSION------------------------------------------


//---------------------------------------------------------------FRIEND INVITES------------------------------------------

void UAGN_SessionSubsystem::InviteFriendToSession(FUniqueNetIdRef FriendNetIdPtr)
{
}

void UAGN_SessionSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int ControllerId, TSharedPtr<const FUniqueNetId, ESPMode::ThreadSafe> UserId, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
}

//---------------------------------------------------------------FRIEND INVITES------------------------------------------