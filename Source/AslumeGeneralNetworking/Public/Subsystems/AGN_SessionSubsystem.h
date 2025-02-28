// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "Headers/AGN_SessionState.h"

#include "AGN_SessionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ASLUMEGENERALNETWORKING_API UAGN_SessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	/**
	Whether the user must stay idle when leaving session
	*/
	bool bShouldStayIdle = true;

	IOnlineSessionPtr SessionInterface = nullptr;

	/**
	Found URL to be joined. Normally empty
	*/
	FString ConnectInfo = "";

	/**
	Found search result to be joined. Normally empty
	*/
	FOnlineSessionSearchResult CurrentSessionToJoin;

	/**
	The current user's session state [Idle, Hosting, Joined]
	*/
	EAGN_HostingState LocalSessionState = EAGN_HostingState::Idle;

	/**
	* Determines whether the subsystem should be creted. Natively turns of the creation unless the class is blueprint successor
	*
	* @param Outer The onwning object of the subsystem
	*/
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/**
	* Setting up the default inbound delegates
	*
	* @param Collection - The onwning object of the subsystem
	*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	Cleaning up the default inbound delegates
	* 
	*/
	virtual void Deinitialize() override;

	/**
	----------Potentially Unnecessary--------
	Registering all local players at the start of the session
	* 
	*/
	virtual void RegisterExistingPlayers();

	/**
	* Callback function called when the session creation process finishes
	* 
	* @param SessionName - The name of the session whose creation was attempted
	* @param bWasSuccessful - Whether the creation was successful
	*/
	virtual void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Callback function called when the session destruction process finishes
	*
	* @param SessionName - The name of the session whose destruction was attempted
	* @param bWasSuccessful - Whether the destruction was successful
	*/
	virtual void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Callback function called when the session joining process finishes
	*
	* @param SessionName - The name of the session whose joining attempt was finished
	* @param JoinResult - The join result: 	
	* [Success, SessionIsFull, SessionDoesNotExist, CouldNotRetrieveAddress, AlreadyInSession, UnknownError]
	*/
	virtual void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	/**
	* Callback function called when the friend invite accepting process finishes
	*
	* @param bWasSuccessful - Whether the accepting was successful
	* @param ControllerId - The ID which will be assigned to you
	* @param UserId - The friend's user ID
	* @param OnlineSessionSearchResult - The session result whose invite was accepted
	*/
	virtual void OnSessionUserInviteAccepted(bool bWasSuccessful, int ControllerId, TSharedPtr<const FUniqueNetId, ESPMode::ThreadSafe> UserId, const FOnlineSessionSearchResult& OnlineSessionSearchResult);

	
public:

	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Maps")
	FString LobbyMapName = "ThirdPersonMap";

	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Maps")
	FString TransitionLoadingMap = "TransitionMap";

	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Maps")
	FString MainMenuMap = "EmptyMapo";

	/*
	UPROPERTY(EditDefaultsOnly, Category = "SessionCoverUp")
	FString LeavingSessionCoverUpMessage = "LeavingSession";

	UPROPERTY(EditDefaultsOnly, Category = "SessionCoverUp")
	FString CreatingSessionCoverUpMessage = "CreatingSession";

	UPROPERTY(EditDefaultsOnly, Category = "SessionCoverUp")
	FString JoinSessionCoverUpMessage = "JoiningSession";
	*/

	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Connection")
	FName GameSessionLocalName = "GameSessionLocalName";

	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Connection")
	int ConnectionPort = 7779;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	int NumPublicConnections = 8;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	int NumPrivateConnections = 0;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bIsLANMatch = false;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bShouldAdvertise = false;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bUsesPresence = true;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bUseLobbiesIfAvailable = false;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bAllowInvites = true;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bAllowJoinViaPresence = false;

	UPROPERTY(EditDefaultsOnly, Category = "SessionSettings")
	bool bAllowJoinInProgress = false;

	/**
	 Attempt create session with the default parameters set in the subsystem
	*
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool DefaultCreateSession();

	/**
	* Attempt create session with the given parameters
	*
	* @param HostingPlayerNum - The LocalPayer num to create the session [typically 0]
	* @param SessionName - The name of the session beeing created
	* 
	* @param InNumPublicConnections - The number of publicly available slots in the session 
	* @param InNumPrivateConnections - The number of privately available slots in the session
	* @param InbIsLANMatch - Wether the match is hosted in the local network only
	* @param InbShouldAdvertise - honestly idfk
	* @param InbUsesPresence Wether - the session will show up as a status message for the players
	* @param InbUseLobbiesIfAvailable - honestly idfk
	* @param InbAllowJoinViaPresence - Wether the session will be joinable from status messages
	* @param InbAllowJoinInProgress - Wether the session will be joinable while it is alredy started
	* 
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool CreateSession(int32 HostingPlayerNum, FName SessionName, int InNumPublicConnections, int InNumPrivateConnections, bool InbIsLANMatch, bool InbShouldAdvertise, bool InbUsesPresence, bool InbUseLobbiesIfAvailable, bool InbAllowInvites, bool InbAllowJoinViaPresence, bool InbAllowJoinInProgress);

	/**
	* Attempt joining the session currently saved in {FOnlineSessionSearchResult CurrentSessionToJoin}
	*
	* @param PlayerNum - The LocalPayer num to join the session [typically 0]
	* @param SessionName - The name of the session to join
	* 
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool JoinSession(int32 PlayerNum, FName SessionName);

	/**
	* Attempt destroying the current local session
	*
	* @param InbShouldStayIdle - Whether the player should stay idle or automatically host a session
	*
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool DestroySession(bool InbShouldStayIdle = true);

	UFUNCTION(BlueprintCallable)
	virtual void RegisterNewPlayer(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable)
	virtual void UnRegisterPlayer(APlayerController* InPlayerController);

	virtual void InviteFriendToSession(FUniqueNetIdRef FriendNetIdPtr);
};
