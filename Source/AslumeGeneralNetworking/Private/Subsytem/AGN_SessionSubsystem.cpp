// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#include "Subsystems/AGN_SessionSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Subsystems/AGN_IdentitySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(AslumeGeneralNetwoking_SessionSubsystem, Log, All);

//---------------------------------------------------------------INITIALIZATION------------------------------------------

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

bool UAGN_SessionSubsystem::DefaultCreateSession(FName SessionName, FString HostName)
{
	return CreateSession(0, SessionName, HostName, DefaultSessionSettings);
}

bool UAGN_SessionSubsystem::CreateSession(int32 HostingPlayerNum, FName SessionName, FString HostName, FSessionSettingsWrapper SessionSettings)
{
	FOnlineSessionSettings SessionSettingsRef = SessionSettings.Get();

	// You *must* set at least one setting value, because you can not run FindSessions without any filters.

	//SessionSettingsRef

	SessionSettingsRef.Settings.Add(
		FName("HostName"),
		FOnlineSessionSetting(HostName, EOnlineDataAdvertisementType::ViaOnlineService));

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleCreateSessionComplete);

	if (!SessionInterface->CreateSession(0, SessionName, SessionSettingsRef))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Creation of session: %s cloud not be started..."), *SessionName.ToString());

		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

		return false;
	}

	return true;
}

void UAGN_SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{

	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
	OnCreateSessionCompleted(SessionName, bWasSuccessful);
}

void UAGN_SessionSubsystem::OnCreateSessionCompleted_Implementation(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		//fire off error delegate for this shit
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Session: %s cloud NOT be created"), *SessionName.ToString());
		return;
	}

	UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Created Session: %s"), *SessionName.ToString());

	LocalHostingState = EAGN_HostingState::Hosting;
}
//---------------------------------------------------------------CREATING SESSION------------------------------------------



//---------------------------------------------------------------FINDING SESSION------------------------------------------

bool UAGN_SessionSubsystem::FindSessions(FSessionSearchWrapper SearchSettings)
{
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleFindSessionsComplete);

	TSharedRef<FOnlineSessionSearch> SearchSettingsPtr = MakeShared<FOnlineSessionSearch>(SearchSettings.Get());

	if (!SessionInterface->FindSessions(0, SearchSettingsPtr))
	{
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Searching for sessions cloud not be started..."));

		SessionInterface->ClearOnFindSessionsCompleteDelegates(this);

		return false;
	}

	return true;
}

void UAGN_SessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
	OnFindSessionsCompleted(bWasSuccessful);
}

void UAGN_SessionSubsystem::OnFindSessionsCompleted_Implementation(bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		//fire off error delegate for this shit
		UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Error, TEXT("Session Search failed"));
		return;
	}

	UE_LOG(AslumeGeneralNetwoking_SessionSubsystem, Display, TEXT("Found some sessions"));
}
//---------------------------------------------------------------FINDING SESSION------------------------------------------



//---------------------------------------------------------------JOINING SESSION------------------------------------------

bool UAGN_SessionSubsystem::JoinSession(int32 PlayerNum, FName SessionName)
{
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UAGN_SessionSubsystem::HandleJoinSessionComplete);
	return false;
}

void UAGN_SessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
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