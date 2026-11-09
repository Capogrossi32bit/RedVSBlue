// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "TeamTypes.h"
#include "TeamPlayerStart.generated.h"

UCLASS()
class REDVSBLUE_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()	

public:

	ATeamPlayerStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	ETeam Team = ETeam::None;

};
