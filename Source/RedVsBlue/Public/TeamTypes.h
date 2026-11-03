// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamTypes.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None UMETA(DisplayName = "None"),
	Blue UMETA(DisplayName = "Blue"),
	Red UMETA(DisplayName = "Red")
};