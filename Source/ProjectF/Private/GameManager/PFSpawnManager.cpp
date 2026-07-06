// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFSpawnManager.h"
#include "GameManager/PFPortalActor.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APFSpawnManager::APFSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}






void APFSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundActors;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APFPortalActor::StaticClass(), FoundActors);
		
	for (AActor* Actor : FoundActors)
	{
		APFPortalActor* Portal = Cast<APFPortalActor>(Actor);
		if (Portal)
		{
			PFPortalActors.Add(Portal);
		}
	}

}





void APFSpawnManager::SpawnWaveMonsters(const TArray<FMonsterSpawnInfo>& SpawnList)
{
	// [바깥쪽 루프]: 영수증에 적힌 몬스터 종류를 하나씩 꺼내서 확인합니다.
	if (PFPortalActors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("PFSpawnManager: 월드에 배치된 PFPortalActor가 없습니다!"));
		return;
	}

	// 포탈들을 순서대로 골고루 쓰기 위한 인덱스 변수
	int32 PortalIndex = 0;

	// [바깥쪽 루프]: 기획서(SpawnList)에서 몬스터 종류를 하나씩 꺼냅니다. (예: 느린 좀비)
	for (const FMonsterSpawnInfo& SpawnInfo : SpawnList)
	{
		if (SpawnInfo.MonsterClass == nullptr) continue;
		for (int32 i = 0; i < SpawnInfo.SpawnCount; ++i)
		{
			APFPortalActor* TargetPortal = PFPortalActors[PortalIndex];
			if (TargetPortal)
			{
				TSubclassOf<APawn> PawnClass = Cast<UClass>(SpawnInfo.MonsterClass);
				
				TargetPortal->SpawnMonster(PawnClass);
			}
			PortalIndex = (PortalIndex + 1) % PFPortalActors.Num();
		}
		UE_LOG(LogTemp, Log, TEXT("PFSpawnManager: %s 몬스터 %d마리 스폰 완료!"), 
			*SpawnInfo.MonsterClass->GetName(), SpawnInfo.SpawnCount);
	}
}


