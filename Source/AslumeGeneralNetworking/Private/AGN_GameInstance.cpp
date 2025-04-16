// Copyright (c) 2025, Aslume Entertainment. All rights reserved.


#include "AGN_GameInstance.h"

#include "Subsystems/AGN_SessionSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(AslumeGeneralNetwoking_GameInstance, Log, All);

void UAGN_GameInstance::OnStart()
{
	Super::OnStart();

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UAGN_GameInstance::HandleNetworkFailure);
	}

	BlueprintInitialization();
}

void UAGN_GameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(AslumeGeneralNetwoking_GameInstance, Warning, TEXT("--------------------------------------------------NETWORK FAILURE-----------------------------------------"));
	UE_LOG(AslumeGeneralNetwoking_GameInstance, Error, TEXT("Error: %s"), *ErrorString);

	UAGN_SessionSubsystem* SessionSubsystemPtr = GetSubsystem<UAGN_SessionSubsystem>();
	if (IsValid(SessionSubsystemPtr) == false)
	{
		UE_LOG(AslumeGeneralNetwoking_GameInstance, Error, TEXT("CreateSession - SessionSubsystem Not Configured Properly"));
		return;
	}

	SessionSubsystemPtr->DestroySession();
}
