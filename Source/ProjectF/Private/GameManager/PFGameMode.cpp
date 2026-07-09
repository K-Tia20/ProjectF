// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFGameMode.h"
#include "GameManager/PFGameState.h"
#include "GameManager/PFPlayerController.h"
#include "GameManager/PFWaveManager.h"
#include "GameManager/PFSpawnManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameManager/PFLevelPortal.h"
#include "Blueprint/UserWidget.h"
#include "Misc/OutputDeviceNull.h"


APFGameMode::APFGameMode()
{
	GameStateClass = APFGameState::StaticClass();
}





void APFGameMode::OnMonsterKilled()
{
	if (!PFGameState) return;
	PFGameState->OnMonsterKilled();
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
	
	{
		PFWaveManager = GetWorld()->SpawnActor<APFWaveManager>(APFWaveManager::StaticClass());
		PFSpawnManager = GetWorld()->SpawnActor<APFSpawnManager>(APFSpawnManager::StaticClass());
		if (PFWaveManager && WaveDataTableAsset)
		{
			PFWaveManager->SetWaveDataTable(WaveDataTableAsset); 
		}
	}
	
	if (PFGameState)
	{
		GetWorldTimerManager().SetTimer(PFTimerHandle, this, &APFGameMode::TimerCountDown, 0.1f, true);
		StartWave();
	}
}





void APFGameMode::TimerCountDown()
{
	if (PFGameState == nullptr) return;
	float NextTime = PFGameState->GetRemainingTime() - 0.1;
	PFGameState->SetRemainingTime(NextTime);

	PFGameState->UpdateHUD();

	if (NextTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(PFTimerHandle);
		GameOver();
	}
}





void APFGameMode::StartWave()
{
	if (!PFGameState || !PFWaveManager || !PFSpawnManager) return;

	int32 CurLevel = PFGameState->GetCurrentLevel();
	int32 CurWave = PFGameState->GetCurrentWave();

		PFWaveManager->CheckAndSpawnWave(CurLevel, CurWave, PFSpawnManager);
	
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

	if (LevelPortalClass)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;

		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			SpawnLocation = PlayerPawn->GetActorLocation() + (PlayerPawn->GetActorForwardVector() * 300.0f);
			SpawnLocation.Z = PlayerPawn->GetActorLocation().Z;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<APFLevelPortal>(LevelPortalClass, SpawnLocation, SpawnRotation, SpawnParams);

		UE_LOG(LogTemp, Warning, TEXT("웨이브 최종 클리어! 탈출 포탈이 생성되었습니다. [F]키로 상호작용하세요."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode에 LevelPortalClass(포탈 블루프린트)가 할당되지 않았습니다!"));
	}
}




void APFGameMode::ProceedToNextLevel()
{
	if (!PFGameState) return;

	int32 NextLevelNum = PFGameState->GetCurrentLevel() + 1;

	if (NextLevelNum <= 4)
	{
		PFGameState->SetCurrentLevel(NextLevelNum);
		PFGameState->SetCurrentWave(1); 
        
		UE_LOG(LogTemp, Warning, TEXT("Level %d 클리어! 다음 레벨로 넘어갑니다."), NextLevelNum - 1);
		if (LoadingWidgetClass)
		{
			UUserWidget* LoadingScreen = CreateWidget<UUserWidget>(GetWorld(), LoadingWidgetClass);
			if (LoadingScreen)
			{
				LoadingScreen->AddToViewport(9999);
				FOutputDeviceNull Ar;
				LoadingScreen->CallFunctionByNameWithArguments(TEXT("PlayFadeOut"), Ar, nullptr, true);
			}
		}
		FString LevelNameString = FString::Printf(TEXT("L_Stage%d"), NextLevelNum);
		FName NextLevelName = FName(*LevelNameString);
		
		UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
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

	if (APFPlayerController* PC = Cast<APFPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ShowGameOver(false);
	}
}