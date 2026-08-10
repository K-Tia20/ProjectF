// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFWaveManager.h"
#include "GameManager/PFSpawnManager.h"
#include "GameManager/PFGameState.h"
#include "Kismet/GameplayStatics.h"



APFWaveManager::APFWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
	
}

void APFWaveManager::BeginPlay()
{
	Super::BeginPlay();
}


void APFWaveManager::CheckAndSpawnWave(int32 CurrentLevel, int32 CurrentWave, class APFSpawnManager* InSpawnManager)
{
	if (PFWaveDataTable == nullptr || InSpawnManager == nullptr) return;

	TArray<FName> RowNames = PFWaveDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FPFWaveData* WaveData = PFWaveDataTable->FindRow<FPFWaveData>(RowName, TEXT(""));

		if (WaveData)
		{
			if (WaveData->Level == CurrentLevel && WaveData->Wave == CurrentWave)
			{
				int32 RunningTotal = 0;
				for (const FMonsterSpawnInfo& MonsterInfo : WaveData->MonsterSpawnList)
				{
					RunningTotal += MonsterInfo.SpawnCount;
				}

				UE_LOG(LogTemp, Log, TEXT("데이터 테이블 매칭 성공! 레벨: %d, 웨이브: %d, 총 스폰수(자동연산): %d"), 
				   CurrentLevel, CurrentWave, RunningTotal);

				if (APFGameState* PFGameState = GetWorld()->GetGameState<APFGameState>())
				{
					PFGameState->SetRemainingMonsters(RunningTotal);
				}

				InSpawnManager->SpawnWaveMonsters(WaveData->MonsterSpawnList);
				return; 
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("PFWaveManager: 레벨 %d, 웨이브 %d에 해당하는 데이터가 테이블에 없습니다."), CurrentLevel, CurrentWave);
}
