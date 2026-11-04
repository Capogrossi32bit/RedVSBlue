#pragma once

#include "CoreMinimal.h"
#include "Flagstate.generated.h"

UENUM(BlueprintType)
enum class EFlagState : uint8
{
	AtBase UMETA(DisplayName = "At Base"),
	Carried UMETA(DisplayName = "Carried"),
	Dropped UMETA(DisplayName = "Dropped")
 };