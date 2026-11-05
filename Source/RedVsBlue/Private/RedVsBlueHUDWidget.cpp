#include "RedVsBlueHUDWidget.h"

#include "RedVsBlueGameState.h"
#include "RedVsBluePlayerState.h"
#include "MatchPhase.h"
#include "TeamTypes.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void URedVsBlueHUDWidget::RefreshHUD()
{
	if (!IsValid(GetWorld()))
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	BlueScoreText->SetText(FText::AsNumber(RBGameState->GetBlueScore()));

	RedScoreText->SetText(FText::AsNumber(RBGameState->GetRedScore()));

	// Tiempo restante

	const int32 RemainingSeconds = RBGameState->GetRemainingTime();

	const int32 Minutes = RemainingSeconds / 60;
	const int32 Seconds = RemainingSeconds % 60;

	const FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	TimeText->SetText(FText::FromString(TimeString));

	// Partida

	const EMatchPhase CurrentPhase = RBGameState->GetMatchPhase();

	FString PhaseString;

	switch (CurrentPhase)
	{
	case EMatchPhase::Waiting:
		PhaseString = TEXT("PREPARACION");
		break;

	case EMatchPhase::Playing:
		PhaseString = TEXT("JUGANDO");
		break;

	case EMatchPhase::Finished:
		PhaseString = TEXT("FINALIZADO");
		break;

	default:
		PhaseString = TEXT("DESCONOCIDO");
		break;

	}

	PhaseText->SetText(FText::FromString(PhaseString));

	//Equipos

	ETeam LocalTeam = ETeam::None;

	APlayerController* PC = GetOwningPlayer();

	if (IsValid(PC))
	{
		ARedVsBluePlayerState* RBPlayerState = PC->GetPlayerState<ARedVsBluePlayerState>();
		if (IsValid(RBPlayerState))
		{
			LocalTeam = RBPlayerState->GetTeam();
		}
	}

	FString TeamString = TEXT("SIN EQUIPO");

	if (LocalTeam == ETeam::Blue)
	{
		TeamString = TEXT("TEAM BLUE");
	}
	else if (LocalTeam == ETeam::Red)
	{
		TeamString = TEXT("TEAM RED");
	}

	TeamText->SetText(FText::FromString(TeamString));

	//Resultado


	if (CurrentPhase != EMatchPhase::Finished)
	{
		ResultText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ResultText->SetVisibility(ESlateVisibility::Visible);

	const ETeam Winner = RBGameState->GetWinner();

	FString ResultString;

	if (Winner == ETeam::None)
	{
		ResultString = TEXT("EMPATE");
	}
	else if (Winner == LocalTeam)
	{
		ResultString = TEXT("VICTORIA");
	}
	else
	{
		ResultString = TEXT("DERROTA");
	}

	ResultText->SetText(FText::FromString(ResultString));
}
