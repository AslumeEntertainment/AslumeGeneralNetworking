// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/OnlineSessionInterface.h"

#include "Headers/AGN_SessionState.h"
#include "Headers/AGN_BlueprintWrapperStructs.h"

#include "AGN_SessionSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionCompletedSignature, FName, SessionName, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFindSessionsCompletedSignature, const TArray<FOnlineSessionSearchResultWrapper>&, Sessions, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionCompletedSignature, bool, bWasSuccessful);

/**
 * 
 */
UCLASS(Blueprintable)
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
	The search settings for the search that is currently going
	*/
	TSharedPtr<class FOnlineSessionSearch> CurrentSearchSettings;

	/**
	The connection info for the current joining session
	*/
	FString CurrentJoiningSessionConnectInfo;

	/**
	Used for nameing the local session in order to 
	*/
	FName GameSessionLocalName = "AGN_LOCAL_SESSION";

	/**
	The current user's session state [Idle, Hosting, Joined]
	*/
	EAGN_HostingState LocalHostingState = EAGN_HostingState::Idle;

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
	* Callback function called when the session creation process finishes
	* 
	* @param SessionName - The name of the session whose creation was attempted
	* @param bWasSuccessful - Whether the creation was successful
	*/
	virtual void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Function used to catch the finish result after CreateSession attempt and pass execution to OnCreateSessionCompleted
	* 
	* @param NOTE: All params are passed directly to  OnCreateSessionCompleted
	*/
	virtual void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Function used to catch the finish result after FindSessions attempt and pass execution to OnFindSessionsCompleted
	*
	* @param NOTE: All params are passed directly to  OnFindSessionsCompleted
	*/
	virtual void HandleFindSessionsComplete(bool bWasSuccessful);

	/**
	* Callback function called when the session joining process finishes
	*
	* @param SessionName - The name of the session whose joining attempt was finished
	* @param JoinResult - The join result: 	
	* [Success, SessionIsFull, SessionDoesNotExist, CouldNotRetrieveAddress, AlreadyInSession, UnknownError]
	*/
	virtual void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	/**
	* Registers the player in the given Session. Automatically called when OnSessionParticipantJoined is triggred
	*
	* @param SessionName - The session in which the player will be registered
	* @param PlayerID - The joining participant's ID
	*/
	virtual void RegisterNewPlayer(FName SessionName, const FUniqueNetId& PlayerID);

	/**
	 * Fired when the player registration process has completed
	 *
	 * @param SessionName - The name of the session the player joined or not
	 * @param Players - The players that were registered from the online service
	 * @param bWasSuccessful - True if the async action completed without error, false if there was an error
	 */
	virtual void HandleRegisterNewPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful);

	/**
	* Unnegisters the player in the given Session. Automatically called when OnSessionParticipantLeft is triggred
	*
	* @param SessionName - The session in which the player will be registered
	* @param PlayerID - The joining participant's ID
	* @param Reason - The reason for disconnect [Left, Disconnected, Kicked, Closed]
	*/
	virtual void UnRegisterPlayer(FName SessionName, const FUniqueNetId& PlayerID, EOnSessionParticipantLeftReason Reason);

	/**
	 * Fired when the un-registration process has completed
	 *
	 * @param SessionName - The name of the session the player left
	 * @param Players - The players that were unregistered from the online service
	 * @param bWasSuccessful - True if the async action completed without error, false if there was an error
	 */
	virtual void HandleUnRegisterPlayerComplete(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasSuccessful);

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

	static const FName GameIDSetting;
	static const FName SessionNameSetting;

	static TArray<FOnlineSessionSearchResultWrapper> WrapSearchResults(TArray<FOnlineSessionSearchResult> FoundSessions);

	/**
	 Used for basic diferentaition between the sessions of different games
	*
	* @return Wether the attempt was started
	*/
	UPROPERTY(EditDefaultsOnly, Category = "ServerSettings | Connection")
	FString GameID = "AGN";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SessionSettings | Creating")
	FSessionSettingsWrapper DefaultSessionSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SessionSettings | Searching");
	FSessionSearchWrapper DefaultSearchSettings;

	/**
	 Attempt create session with the default parameters set in the subsystem
	*
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool DefaultCreateSession(FString SessionName);

	/**
	* Attempt create session with the given parameters
	*
	* @param HostingPlayerNum - The LocalPayer num to create the session [typically 0]
	* @param SessionName - The name of the session beeing created
	* @param HostName - The name of the player creating the session [Used as native Setting in SessionSettings]
	* 
	* @param SessionSettings - list of settings incuding:
	* @param  - InNumPublicConnections - The number of publicly available slots in the session 
	* @param  - InNumPrivateConnections - The number of privately available slots in the session
	* @param  - InbIsLANMatch - Whether the match is hosted in the local network only
	* @param  - InbShouldAdvertise - Whether the session is dicovarable with FindSessions
	* @param  - InbUsesPresence - Whether the session will show up as a status message for the players
	* @param  - InbUseLobbiesIfAvailable - honestly idfk
	* @param  - InbAllowJoinViaPresence - Whether the session will be joinable from status messages
	* @param  - InbAllowJoinInProgress - Whether the session will be joinable while it is alredy started
	* @param  - List of specific session setings sucha as the default HostName
	* 
	* @return Whether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool CreateSession(int32 HostingPlayerNum, FString SessionName, FSessionSettingsWrapper SessionSettings);

	/**
	* Attempt finding sessions with the default SearchSettings
	*
	*
	* @return Whether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool DefautFindSessions();

	/**
	* Attempt finding sessions with the given parameters
	*
	* 
	* @return Whether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool FindSessions(FSessionSearchWrapper SearchSettings);

	/**
	* Attempt joining the session currently saved in {FOnlineSessionSearchResult CurrentSessionToJoin}
	*
	* @param PlayerNum - The LocalPayer num to join the session [typically 0]
	* @param SessionName - The name of the session to join
	* 
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool JoinSession(int32 PlayerNum, FOnlineSessionSearchResultWrapper Session);

	/**
	* Attempt destroying the current local session
	*
	* @param InbShouldStayIdle - Whether the player should stay idle or automatically host a session
	*
	* @return Wether the attempt was started
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool DestroySession(bool InbShouldStayIdle = true);

	virtual void InviteFriendToSession(FUniqueNetIdRef FriendNetIdPtr);

	/**
	* Delegate called after CreateSession attempt finishes
	* Assignable in blueprints and c++
	*
	* @param SessionName - The name of the session whose destruction was attempted
	* @param bWasSuccessful - Whether the creation was successful
	*/
	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionCompletedSignature OnCreateSessionCompleted;

	/**
	* Delegate called after FindSessions attempt finishes
	* Assignable in blueprints and c++
	*
	* @param Sessions - All the found session in blueprint digestable format
	* @param bWasSuccessful - Whether the search was successful
	*/
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionsCompletedSignature OnFindSessionsCompleted;

	/**
	* Delegate called after JoinSession attempt finishes
	* Assignable in blueprints and c++
	*
	* @param bWasSuccessful - Whether the search was successful
	*/
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionCompletedSignature OnJoinSessionCompleted;
};
