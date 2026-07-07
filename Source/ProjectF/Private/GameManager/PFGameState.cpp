// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFGameState.h"
#include "GameManager/PFPlayerController.h"
#include "GameManager/PFGameMode.h"

APFGameState::APFGameState()
{
	CurrentLevel = 1;
	CurrentWave = 1;
	RemainingMonsters = 0;
	RemainingTime = 300.0f;
}





void APFGameState::UpdateHUD()
{
	APFPlayerController* PlayerController = Cast<APFPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController == nullptr) return;
	
	PlayerController->UpdateWaveText(CurrentWave);
	PlayerController->UpdateMonsterText(RemainingMonsters);

	int32 Min = FMath::FloorToInt(RemainingTime / 60.0f);
	int32 Sec = FMath::FloorToInt(FMath::Fmod(RemainingTime, 60.0f));
	
	float SubSeconds = RemainingTime - FMath::FloorToFloat(RemainingTime);
	int32 AbsoluteMs = FMath::FloorToInt(SubSeconds * 100.0f);
	int32 FineMsTail = FMath::FloorToInt(SubSeconds * 1000.0f) % 10;
	int32 FinalMs = (AbsoluteMs - (AbsoluteMs % 10)) + FineMsTail;
	
	if (FinalMs >= 100) FinalMs = 99;
	if (FinalMs < 0) FinalMs = 0;
	
	PlayerController->UpdateTimer(Min, Sec, FinalMs);
}


void APFGameState::OnMonsterKilled()
{
	RemainingMonsters = FMath::Max(0, RemainingMonsters - 1);

	if (APFPlayerController* PlayerController = Cast<APFPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->UpdateMonsterText(RemainingMonsters);
	}

	if (RemainingMonsters <= 0)
	{
		if (APFGameMode* GameMode = Cast<APFGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->OnMonsterKilled(); 
		}
	}
}



