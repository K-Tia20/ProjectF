// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFGameMode.h"
#include "GameManager/PFGameState.h"



APFGameMode::APFGameMode()
{
	GameStateClass = APFGameState::StaticClass();
}





void APFGameMode::OnMonsterKilled()
{
	if (!PFGameState) return;
	PFGameState->DecreaseMonsterCount();
    
	UE_LOG(LogTemp, Log, TEXT("몬스터 처치! 남은 몬스터: %d"), PFGameState->GetRemainingMonsters());

	if (PFGameState->GetRemainingMonsters() <= 0)
	{
		WaveClear();
	}
}






void APFGameMode::BeginPlay()
{
	Super::BeginPlay();
	PFGameState = Cast<APFGameState>(GameState);
	if (PFGameState)
	{
		GetWorldTimerManager().SetTimer(PFTimerHandle, this, &APFGameMode::TimerCountDown, 0.1f, false);
		StartWave();
	}
}





void APFGameMode::TimerCountDown()
{
	if (PFGameState == nullptr) return;
	float NextTime = PFGameState->GetRemainingTime() - 0.1;
	PFGameState->SetRemainingTime(NextTime);


	if (NextTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(PFTimerHandle);
		GameOver();
	}
}





void APFGameMode::StartWave()
{
	if (!PFGameState) return;

	int32 CurLevel = PFGameState->GetCurrentLevel();
	int32 CurWave = PFGameState->GetCurrentWave();

	UE_LOG(LogTemp, Log, TEXT("Level %d - Wave %d 시작!"), CurLevel, CurWave);
	PFGameState->SetRemainingMonsters(5); 
}





void APFGameMode::WaveClear()
{
	if (!PFGameState) return;

	int32 NextWave = PFGameState->GetCurrentWave() + 1;

	if (NextWave <= 3)
	{
		PFGameState->SetCurrentWave(NextWave);
		StartWave();
	}
	else
	{
		LevelClear();
	}
}





void APFGameMode::LevelClear()
{
	if (!PFGameState) return;

	int32 NextLevel = PFGameState->GetCurrentLevel() + 1;

	if (NextLevel <= 4)
	{
		PFGameState->SetCurrentLevel(NextLevel);
		PFGameState->SetCurrentWave(1);
        
		UE_LOG(LogTemp, Warning, TEXT("Level %d 클리어! 다음 레벨로 넘어갑니다."), NextLevel - 1);
		StartWave(); 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("축하합니다! 모든 레벨을 클리어하여 게임을 승리했습니다!"));
		GetWorldTimerManager().ClearTimer(PFTimerHandle);
	}
}





void APFGameMode::GameOver()
{
	GetWorldTimerManager().ClearTimer(PFTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("Game Over!"));
}