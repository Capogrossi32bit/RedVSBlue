#include "RedVsBlueHUDWidget.h"

#include "RedVsBlueGameState.h"
#include "RedVsBluePlayerState.h"
#include "RedVsBlueCharacter.h"
#include "HealthComponent.h"
#include "MatchPhase.h"
#include "TeamTypes.h"

#include "Components/ProgressBar.h"
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
			
			if (IsValid(KillsText))
			{
				KillsText->SetText(FText::FromString(FString::Printf(TEXT("KILLS: %d"),RBPlayerState->GetKills())));
			}

			if (IsValid(DeathsText))
			{
				DeathsText->SetText(FText::FromString(FString::Printf(TEXT("DEATHS: %d"),RBPlayerState->GetDeaths())));
			}
			
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

	RefreshCombatHUD();

	if (CurrentPhase != EMatchPhase::Finished)
	{
		ResultText->SetVisibility(ESlateVisibility::Collapsed);
		
		if (IsValid(Button_BackToMenu))
		{
			Button_BackToMenu->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	ResultText->SetVisibility(ESlateVisibility::Visible);

	if (IsValid(Button_BackToMenu))
	{
		Button_BackToMenu->SetVisibility(ESlateVisibility::Visible);
	}

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

void URedVsBlueHUDWidget::RefreshCombatHUD()
{

	APlayerController* PC = GetOwningPlayer();

	if (!IsValid(PC))
	{
		return;
	}
	ARedVsBlueCharacter* Character = Cast<ARedVsBlueCharacter>(PC->GetPawn());
	
	if (!IsValid(Character))
	{
		if (IsValid(HealthBar))
		{
			HealthBar->SetPercent(0.f);
		}

		if (IsValid(HealthText))
		{
			HealthText->SetText(FText::FromString(TEXT("-- / -- HP")));
		}

		if (IsValid(AmmoText))
		{
			AmmoText->SetText(FText::FromString(TEXT("-- / --")));
		}

		if (IsValid(ReloadText))
		{
			ReloadText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	UHealthComponent* HealthComponent = Character->FindComponentByClass<UHealthComponent>();

	if (IsValid(HealthComponent))
	{
		const float CurrentHealth = HealthComponent->GetHealth();

		const float MaxHealth = HealthComponent->GetMaxHealth();

		const float HealthPercent = MaxHealth > 0.f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;

		if (IsValid(HealthBar))
		{
			HealthBar->SetPercent(HealthPercent);
		}
		if (IsValid(HealthText))
		{
			HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f HP"), CurrentHealth, MaxHealth)));
		}
	}

	if (IsValid(AmmoText))
	{
		AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Character->GetCurrentAmmo(), Character->GetReserveAmmo())));
	}

	

	if (IsValid(ReloadText))
	{
		const bool bReloading = Character->IsReloading();

		ReloadText->SetVisibility(bReloading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		
		if (bReloading)
		{
			ReloadText->SetText(FText::FromString(TEXT("RECARGANDO...")));
		}
	}
}

void URedVsBlueHUDWidget::RequestReturnMenu()
{
	OnReturnToMenuRequested.Broadcast();
}
