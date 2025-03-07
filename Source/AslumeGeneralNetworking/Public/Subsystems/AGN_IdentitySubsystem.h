// Copyright (c) 2025, Aslume Entertainment. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/OnlineIdentityInterface.h"

#include "AGN_IdentitySubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ASLUMEGENERALNETWORKING_API UAGN_IdentitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:

	IOnlineIdentityPtr IdentityInterface = nullptr;

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
	* Function used to catch the finish result after login attempt and pass execution to OnLoginCompleted
	*
	* @param NOTE: All params are passed directly to  OnLoginCompleted
	*/
	virtual void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

public:

	/**
	* Attempt login with the default online susbsytem
	*
	* @param Id - defined by the delaut systems implementation
	* @param Token - defined by the delaut systems implementation
	* @param Type - defined by the delaut systems implementation
	* 
	* @return Wheter the atempt was started correctly
	*/
	UFUNCTION(BlueprintCallable)
	virtual bool Login(FString Id = "", FString Token = "", FString Type = "");

	/**
	* Function called after login attempt finishes
	* Defineable in blueprints
	* For C++ implementation override the OnLoginCompleted_Implementation
	*
	* @param LocalUserNum - The index of the local user being logged in
	* @param bWasSuccessful - Wheter the login attempt was successful
	* @param UserId - The strign representation of unique NetID of the given user ["" if loggin failed]
	* @param Error - The occured error in login attempt [null if login succeeded]
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnLoginCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString& PlayerName, const FString& Error);

	virtual FUniqueNetIdPtr GetLocalPlayerNetId();

	virtual FString GetLocalPlayerDisplayName();

	UFUNCTION(BlueprintCallable)
	virtual bool IsPlayerLoggedIn();



};
