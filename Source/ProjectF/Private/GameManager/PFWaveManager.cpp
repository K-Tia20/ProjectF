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
	
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), APFSpawnManager::StaticClass());
	PFSpawnManager = Cast<APFSpawnManager>(FoundActor);	
}


void APFWaveManager::CheckAndSpawnWave(int32 CurrentLevel, int32 CurrentWave)
{
	if (PFWaveDataTable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PFWaveManager: 데이터 테이블이 등록되지 않았습니다!"));
		return;
	}
	if (PFSpawnManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PFWaveManager: PFSpawnManager를 찾을 수 없습니다!"));
		return;
	}

	TArray<FName> RowNames = PFWaveDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FPFWaveData* WaveData = PFWaveDataTable->FindRow<FPFWaveData>(RowName, TEXT(""));

		if (WaveData)
		{
			if (WaveData->Level == CurrentLevel && WaveData->Wave == CurrentWave)
			{
				UE_LOG(LogTemp, Log, TEXT("데이터 테이블 매칭 성공! 레벨: %d, 웨이브: %d, 총 스폰수: %d"), 
					CurrentLevel, CurrentWave, WaveData->TotalSpawnCount);
				if (APFGameState* PFGameState = GetWorld()->GetGameState<APFGameState>())
				{
					PFGameState->SetRemainingMonsters(WaveData->TotalSpawnCount);
				}

				PFSpawnManager->SpawnWaveMonsters(WaveData->MonsterSpawnList);
				return; 
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("PFWaveManager: 레벨 %d, 웨이브 %d에 해당하는 데이터가 테이블에 없습니다."), CurrentLevel, CurrentWave);
}
// 1
// asdfasdf