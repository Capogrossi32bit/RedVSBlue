// Fill out your copyright notice in the Description page of Project Settings.


#include "RedVsBlueGameModeBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Flag.h"
#include "FlagState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "MatchPhase.h"
#include "RedVsBlueGameState.h"
#include "RedVsBluePlayerState.h"
#include "RedVsBlueCharacter.h"
#include "TeamPlayerStart.h"
#include "TimerManager.h"

ARedVsBlueGameModeBase::ARedVsBlueGameModeBase()
{
	PlayerStateClass = ARedVsBluePlayerState::StaticClass();

	GameStateClass = ARedVsBlueGameState::StaticClass();
}

void ARedVsBlueGameModeBase::RespawnPlayer(TWeakObjectPtr<APlayerController> PlayerController)
{

	if (!HasAuthority() || !PlayerController.IsValid())
	{
		return;
	}

	APlayerController* PC = PlayerController.Get();
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState) || RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	if (IsValid(PC->GetPawn()))
	{
		return;
	}

	RestartPlayer(PC);
}

void ARedVsBlueGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	StartWaitingPhase();
}

void ARedVsBlueGameModeBase::StartWaitingPhase()
{
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	RBGameState->SetRemainingTime(FMath::Max(0, WaitingDuration));

	RBGameState->SetMatchPhase(EMatchPhase::Waiting);


	if (WaitingDuration <= 0)
	{
		StartPlayingPhase();
		return;
	}

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ARedVsBlueGameModeBase::UpdateMatchTimer, 1.0f, true);

}

AActor* ARedVsBlueGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!IsValid(Player))
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	ARedVsBluePlayerState* RBPlayerState = Player->GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(RBPlayerState))
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}
	const ETeam PlayerTeam = RBPlayerState->GetTeam();

	if (PlayerTeam == ETeam::None)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<ATeamPlayerStart*> AvailableStarts;

	for (TActorIterator<ATeamPlayerStart> It(GetWorld()); It; ++It)
	{
		ATeamPlayerStart* Start = *It;

		if (IsValid(Start) && Start->Team == PlayerTeam)
		{
			AvailableStarts.Add(Start);
		}
	}

	if (AvailableStarts.IsEmpty())
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const int32 RandomIndex = FMath::RandRange(0, AvailableStarts.Num() - 1);

	ATeamPlayerStart* SelectedStart = AvailableStarts[RandomIndex];

	return SelectedStart;
}

AActor* ARedVsBlueGameModeBase::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (!IsValid(Player))
	{
		return Super::FindPlayerStart_Implementation(Player,IncomingName);
	}
	
	if (!IncomingName.IsEmpty())
	{
		return Super::FindPlayerStart_Implementation(Player, IncomingName);
	}

	return ChoosePlayerStart_Implementation(Player);
}

void ARedVsBlueGameModeBase::StartPlayingPhase()
{
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();
	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	RBGameState->SetRemainingTime(FMath::Max(0, MatchDuration));

	RBGameState->SetMatchPhase(EMatchPhase::Playing);


	if (MatchDuration <= 0)
	{
		FinishMatch();
		return;
	}

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ARedVsBlueGameModeBase::UpdateMatchTimer, 1.0f, true);
}

void ARedVsBlueGameModeBase::FinishMatch()
{
	if (!HasAuthority())
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	const int32 BlueScore = RBGameState->GetBlueScore();
	const int32 RedScore = RBGameState->GetRedScore();

	ETeam WinningTeam = ETeam::None;

	if (BlueScore > RedScore)
	{
		WinningTeam = ETeam::Blue;
	}

	else if (RedScore > BlueScore)
	{
		WinningTeam = ETeam::Red;
	}

	RBGameState->SetRemainingTime(0);
	RBGameState->SetWinner(WinningTeam);
	RBGameState->SetMatchPhase(EMatchPhase::Finished);
}

void ARedVsBlueGameModeBase::UpdateMatchTimer()
{
	if (!HasAuthority())
	{
		return;
	}
	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	const int32 NewTime = FMath::Max(0, RBGameState->GetRemainingTime() - 1);

	RBGameState->SetRemainingTime(NewTime);

	if (NewTime > 0)
	{
		return;
	}

	switch (RBGameState->GetMatchPhase())
	{
	case EMatchPhase::Waiting:
		StartPlayingPhase();
		break;

	case EMatchPhase::Playing:
		FinishMatch();
		break;

	case EMatchPhase::Finished:
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		break;
	}
}

void ARedVsBlueGameModeBase::AssignTeam(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		return;
	}

	ARedVsBluePlayerState* NewPlayerState = NewPlayer->GetPlayerState<ARedVsBluePlayerState>();

	if (!IsValid(NewPlayerState))
	{
		return;
	}

	int32 BlueCount = 0;
	int32 RedCount = 0;

	for (auto It = AssignedTeams.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		if (It.Value() == ETeam::Blue)
		{
			BlueCount++;
		}
		else if (It.Value() == ETeam::Red)
		{
			RedCount++;
		}
	}

	if (AssignedTeams.Contains(NewPlayerState))
	{
		if (NewPlayerState->GetTeam() != ETeam::None)
		{
			return;
		}
		AssignedTeams.Remove(NewPlayerState);
	}

	ETeam AssignedTeam = BlueCount <= RedCount ? ETeam::Blue : ETeam::Red;

	NewPlayerState->SetTeam(AssignedTeam);


	AssignedTeams.Add(NewPlayerState, AssignedTeam);

}

void ARedVsBlueGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	APlayerController* PC = Cast<APlayerController>(NewPlayer);
	if (!IsValid(PC))
	{
		return;
	}

	// Asignar el equipo antes del spawn normal.
	AssignTeam(PC);

}

bool ARedVsBlueGameModeBase::TryCaptureFlag(ARedVsBlueCharacter* ScoringCharacter, AFlag* HomeFlag)
{
	if (!HasAuthority())
	{
		return false;
	}

	ARedVsBlueGameState* MatchState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(MatchState) || MatchState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return false;
	}

	if (!IsValid(ScoringCharacter) || !IsValid(HomeFlag))
	{
		return false;
	}

	const ETeam PlayerTeam = ScoringCharacter->GetTeam();

	if (PlayerTeam == ETeam::None)
	{
		return false;
	}
	const FString TeamName = PlayerTeam == ETeam::Blue ? TEXT("Blue") : TEXT("Red");

	if (HomeFlag->Team != TeamName)
	{
		return false;
	}

	if (HomeFlag->GetFlagState() != EFlagState::AtBase)
	{
		return false;
	}

	const float Distance = FVector::Dist(ScoringCharacter->GetActorLocation(), HomeFlag->GetActorLocation());

	if (Distance > 250.f)
	{
		return false;
	}
	
	AFlag* EnemyFlag = ScoringCharacter->GetCarriedFlag();

	if (!IsValid(EnemyFlag) || EnemyFlag == HomeFlag)
	{
		return false;
	}

	if (EnemyFlag->Team == TeamName)
	{
		return false;
	}

	if (EnemyFlag->GetFlagState() != EFlagState::Carried)
	{
		return false;
	}


	if (!IsValid(MatchState))
	{
		return false;
	}

	MatchState->AddScore(PlayerTeam);

	EnemyFlag->ReturnToBase();

	return true;
}

void ARedVsBlueGameModeBase::HandlePlayerDeath(ARedVsBlueCharacter* DeadCharacter)
{
	if (!HasAuthority()||!IsValid(DeadCharacter))
	{
		return;
	}
	APlayerController* VictimController = Cast<APlayerController>(DeadCharacter->GetController());

	if (!IsValid(VictimController))
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	ARedVsBluePlayerState* VictimPlayerState = VictimController->GetPlayerState<ARedVsBluePlayerState>();

	UHealthComponent* HealthComponent = DeadCharacter->FindComponentByClass<UHealthComponent>();

	AController* KillerController = nullptr;

	if (IsValid(HealthComponent))
	{
		KillerController = HealthComponent->GetKillingController();
	}

	ARedVsBluePlayerState* KillerPlayerState = nullptr;
	
	if (IsValid(KillerController))
	{
		KillerPlayerState = KillerController->GetPlayerState<ARedVsBluePlayerState>();
	}

	if (IsValid(VictimPlayerState))
	{
		VictimPlayerState->AddDeath();
	}

	if (IsValid(KillerPlayerState) && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->AddKill();
	}

	//Si no estamos en fase de juego no hacemos nada
	if (RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	RBGameState->Multicast_PlayDeathSound(DeadCharacter->GetActorLocation());

	TWeakObjectPtr<APlayerController> WeakPC = VictimController;

	VictimController->UnPossess();

	DeadCharacter->Destroy();

	if (RBGameState->GetMatchPhase() != EMatchPhase::Playing)
	{
		return;
	}

	//Encargado del conteo para respawnear
	FTimerDelegate RespawnDelegate;

	RespawnDelegate.BindUObject(this, &ARedVsBlueGameModeBase::RespawnPlayer, WeakPC);

	FTimerHandle RespawnTimerHandle;

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);

}
