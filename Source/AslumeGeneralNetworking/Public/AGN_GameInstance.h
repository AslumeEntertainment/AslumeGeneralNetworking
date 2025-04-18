// Copyright(c) 2025, Aslume Entertainment. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AGN_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ASLUMEGENERALNETWORKING_API UAGN_GameInstance : public UGameInstance
{
	GENERATED_BODY()

private:


	/**
	* Automatically called when network error occures. Natively uses the Session subsystem to destroy the active session if one is present
	*
	* @param World The world in wich the network error occured.
	* @param NetDriver The Specific NetDriver used by the current default online subsystem.
	* @param FailureType The type of the error
	* @param ErrorString Detailed network error description
	*/
	virtual void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

protected:

public:

	/**
	* Automatically called when starting the game. Natively binds internal delegates to UAGN_GameInstance::HandleNetworkFailure
	*
	*/
	virtual void OnStart() override;

	/**
	* Intented to be overriden by blueprints
	* for blueprint specific setup and delegate binding
	*
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintInitialization();

	UFUNCTION(BlueprintCallable)
	void ClintTravel(APlayerController* PC, FString URL, ETravelType TravelType, bool Seamless);
};
