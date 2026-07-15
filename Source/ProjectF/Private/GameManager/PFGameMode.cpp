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
#include "Item/PFBaseItem.h"


APFGameMode::APFGameMode()
{
	GameStateClass = APFGameState::StaticClass();
	OverallDropChance = 0.2f;
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

	if (LevelPortalInstance)
	{
		LevelPortalInstance->SetPortalActive(true);
		UE_LOG(LogTemp, Warning, TEXT("웨이브 최종 클리어! 탈출 포탈이 활성화되었습니다. [F]키로 상호작용하세요."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("월드에 배치되어 자동으로 등록된 APFLevelPortal 인스턴스가 없습니다!"));
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
		FString LevelNameString = FString::Printf(TEXT("L_Map%d"), NextLevelNum);
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



void APFGameMode::HandleMonsterDeath(const FVector& SpawnLocation)
{
	int32 CurrentLevel = PFGameState->GetCurrentLevel();
    // 20% 확률
    float RandRoll = FMath::FRand();
    
    if (RandRoll <= OverallDropChance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ItemDrop] 성공 "));
        SpawnRandomItem(SpawnLocation, CurrentLevel);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[ItemDrop] 실패 "));
    }
}

void APFGameMode::SpawnRandomItem(const FVector& SpawnLocation, int32 CurrentLevel)
{
    struct FDropCandidate
    {
        TSubclassOf<APFBaseItem> ItemClass;
        float Weight; // 레벨 1에서 RifleAmmoItem 드랍 안되게 하기 위해 가중치 선언
    };

    TArray<FDropCandidate> ActivePool;

    // 현재 활성화할 아이템
    if (HealingItemClass) ActivePool.Add({ HealingItemClass, 30.f });
    if (PistolAmmoClass)  ActivePool.Add({ PistolAmmoClass, 30.f });
    if (GrenadeItemClass) ActivePool.Add({ GrenadeItemClass, 10.f });

    // 레벨이 2 이상일 때만 라이플 아모 추가
    if (CurrentLevel >= 2 && RifleAmmoClass)
    {
        ActivePool.Add({ RifleAmmoClass, 30.f });
    }
	
    if (ActivePool.Num() == 0) return;

    // 누적 가중치(Total Weight) 계산
    float TotalWeight = 0.f;
    for (const auto& Candidate : ActivePool)
    {
        TotalWeight += Candidate.Weight;
    }

    // 랜덤 선택
    float RandWeight = FMath::FRand() * TotalWeight;
    TSubclassOf<APFBaseItem> SelectedItemClass = nullptr;

    float CumulativeWeight = 0.f;
    for (const auto& Candidate : ActivePool)
    {
        CumulativeWeight += Candidate.Weight;
        if (RandWeight <= CumulativeWeight)
        {
            SelectedItemClass = Candidate.ItemClass;
            break;
        }
    }

    // 아이템 월드 스폰
    if (SelectedItemClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 아이템 스폰 시 바닥에 묻히지 않도록 Z축 설정
        FVector AdjustedLocation = SpawnLocation + FVector(0.f, 0.f, 30.f);

        GetWorld()->SpawnActor<APFBaseItem>(SelectedItemClass, AdjustedLocation, FRotator::ZeroRotator, SpawnParams);
        UE_LOG(LogTemp, Warning, TEXT("[ItemDrop] 아이템 스폰 : %s"), *SelectedItemClass->GetName());
    }
}