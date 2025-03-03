// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "OnlineSessionSettings.h"

#include "AGN_BlueprintWrapperStructs.generated.h"

UENUM(BlueprintType)
enum class EOnlineSessionJoinPolicy : uint8
{
	/** The session will appear on searches an may be joined by anyone */
	Public,

	/** The session will not appear on searches and may only be joined via presence (if enabled) or invitation */
	FriendsOnly,

	/** The session will not appear on searches and may not be joined via presence, only via invitation */
	InviteOnly
};

UENUM(BlueprintType)
enum class EOnlineDataAdvertisementTypeWrapper : uint8
{
	/** Don't advertise via the online service or QoS data */
	DontAdvertise,
	/** Advertise via the server ping data only */
	ViaPingOnly,
	/** Advertise via the online service only */
	ViaOnlineService,
	/** Advertise via the online service and via the ping data */
	ViaOnlineServiceAndPing
};

USTRUCT(BlueprintType)
struct FOnlineSessionSettingWrapper
{
	GENERATED_BODY()

public:
	/** Settings value */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVariantData Data;

	/** How is this session setting advertised with the backend or searches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOnlineDataAdvertisementTypeWrapper AdvertisementType = EOnlineDataAdvertisementTypeWrapper::ViaOnlineService;

	/** Optional ID used in some platforms as the index instead of the session name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID;

	FOnlineSessionSetting Get()
	{
		FOnlineSessionSetting Value = FOnlineSessionSetting();
		Value.ID = ID;

		switch (AdvertisementType)
		{
		case EOnlineDataAdvertisementTypeWrapper::ViaPingOnly:
			Value.AdvertisementType = EOnlineDataAdvertisementType::ViaPingOnly; break;

		case EOnlineDataAdvertisementTypeWrapper::ViaOnlineService:
			Value.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService; break;

		case EOnlineDataAdvertisementTypeWrapper::ViaOnlineServiceAndPing:
			Value.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineServiceAndPing; break;

		default:
			Value.AdvertisementType = EOnlineDataAdvertisementType::DontAdvertise; break;
		}

		Value.Data = FVariantData();
		return Value;
	}
};

/**
 Direct blueprintable wrapper for FOnlineSessionSettings
*/
USTRUCT(BlueprintType)
struct FSessionSettingsWrapper
{
	GENERATED_BODY()

	/** The join policy [Public, FrientsOnly, InvitesOnly] |Currently Does nothing| */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOnlineSessionJoinPolicy JoinPolicy = EOnlineSessionJoinPolicy::Public;

	/** The number of publicly available connections advertised */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPublicConnections = 8;

	/** The number of connections that are private (invite/password) only */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPrivateConnections = 0;

	/** Whether this match is publicly advertised on the online service */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldAdvertise = true;

	/** Whether joining in progress is allowed or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowJoinInProgress = false;

	/** This game will be lan only and not be visible to external players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLANMatch = false;

	/** Whether the server is dedicated or player hosted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDedicated = false;

	/** Whether the match should gather stats or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsesStats = false;

	/** Whether the match allows invitations for this session or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowInvites = true;

	/** Whether to display user presence information or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsesPresence = true;

	/** Whether joining via player presence is allowed or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowJoinViaPresence = false;

	/** Whether joining via player presence is allowed for friends only or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowJoinViaPresenceFriendsOnly = true;

	/** Whether the server employs anti-cheat (punkbuster, vac, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAntiCheatProtected = false;

	/** Whether to prefer lobbies APIs if the platform supports them */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseLobbiesIfAvailable = false;

	/** Whether to create (and auto join) a voice chat room for the lobby, if the platform supports it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseLobbiesVoiceChatIfAvailable = false;

	/** Manual override for the Session Id instead of having one assigned by the backend. Its size may be restricted depending on the platform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionIdOverride;

	/** Used to keep different builds from seeing each other during searches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BuildUniqueId = 1001;

	/** Array of custom session settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FOnlineSessionSettingWrapper> Settings;

	/** Map of custom settings per session member (Not currently used by every OSS) [Handled at runtime] */
	TUniqueNetIdMap<FSessionSettings> MemberSettings;

	FOnlineSessionSettings Get()
	{
		FOnlineSessionSettings Value = FOnlineSessionSettings();
		Value.NumPublicConnections = NumPublicConnections;
		Value.NumPrivateConnections = NumPrivateConnections;
		Value.bShouldAdvertise = bShouldAdvertise;
		Value.bAllowJoinInProgress = bAllowJoinInProgress;
		Value.bIsLANMatch = bIsLANMatch;
		Value.bIsDedicated = bIsDedicated;
		Value.bUsesStats = bUsesStats;
		Value.bAllowInvites = bAllowInvites;
		Value.bUsesPresence = bUsesPresence;
		Value.bAllowJoinViaPresence = bAllowJoinViaPresence;
		Value.bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;
		Value.bAntiCheatProtected = bAntiCheatProtected;
		Value.bUseLobbiesIfAvailable = bUseLobbiesIfAvailable;
		Value.bUseLobbiesVoiceChatIfAvailable = bUseLobbiesVoiceChatIfAvailable;
		Value.SessionIdOverride = SessionIdOverride;

		Value.BuildUniqueId = BuildUniqueId;
		Value.Settings = FSessionSettings();
		for (TPair<FName, FOnlineSessionSettingWrapper> Setting : Settings)
		{
			Value.Settings.Add(Setting.Key, Setting.Value.Get());
		}
		Value.MemberSettings = MemberSettings;

		return Value;
	}
};


/**
 Direct blueprintable wrapper for FOnlineSessionSearch
*/
USTRUCT(BlueprintType)
struct FSessionSearchWrapper
{
	GENERATED_BODY()

	/** Array of all sessions found when searching for the given criteria */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FOnlineSessionSearchResult> SearchResults;

	/** The query to use for finding matching servers */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOnlineSearchSettings QuerySettings;

	/** State of the search */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EOnlineAsyncTaskState::Type SearchState = EOnlineAsyncTaskState::NotStarted;

	/** Max number of queries returned by the matchmaking service */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSearchResults = 99;

	/** Whether the query is intended for LAN matches or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLanQuery = false;

	/**
	 * Used to sort games into buckets since a the difference in terms of feel for ping
	 * in the same bucket is often not a useful comparison and skill is better
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PingBucketSize = 10;

	/** Search hash used by the online subsystem to disambiguate search queries, stamped every time FindSession is called */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatformHash = 0;

	/** Amount of time to wait for the search results. May not apply to all platforms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeoutInSeconds = 4;

	FOnlineSessionSearch Get()
	{
		FOnlineSessionSearch Value = FOnlineSessionSearch();
		Value.bIsLanQuery = bIsLanQuery;
		Value.PingBucketSize = PingBucketSize;
		Value.MaxSearchResults = MaxSearchResults;
		Value.PlatformHash = PlatformHash;
		Value.QuerySettings = QuerySettings;
		Value.SearchResults = SearchResults;
		Value.SearchState = SearchState;
		Value.TimeoutInSeconds = TimeoutInSeconds;
		return Value;
	}

};