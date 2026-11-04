#pragma once

#include "CoreMinimal.h"
#include "MatchPhase.generated.h"

UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Playing UMETA(DisplayName = "Playing"),
	Finished UMETA(DisplayName = "Finished")
};