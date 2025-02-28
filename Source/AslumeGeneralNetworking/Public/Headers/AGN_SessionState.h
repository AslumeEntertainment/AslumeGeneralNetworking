#pragma once

#include "CoreMinimal.h"
#include "AGN_SessionState.generated.h"

UENUM(BlueprintType)
enum class EAGN_HostingState : uint8
{
	Hosting,
	Joined,
	Idle,
};
