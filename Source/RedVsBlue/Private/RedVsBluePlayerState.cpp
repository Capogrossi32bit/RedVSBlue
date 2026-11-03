// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBluePlayerState.h"
#include "Net/UnrealNetwork.h"

ETeam ARedVsBluePlayerState::GetTeam() const
{
	return Team;
}

void ARedVsBluePlayerState::SetTeam(ETeam NewTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	Team = NewTeam;
	OnRep_Team();
}

void ARedVsBluePlayerState::OnRep_Team()
{
	const FString TeamText = StaticEnum<ETeam>()->GetNameStringByValue(static_cast<int64>(Team));

}


void ARedVsBluePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARedVsBluePlayerState, Team);
}



