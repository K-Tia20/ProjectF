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
	PFPortalActors.Empty();
	BossPortalActors.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APFPortalActor::StaticClass(), FoundActors);
        
	for (AActor* Actor : FoundActors)
	{
		APFPortalActor* Portal = Cast<APFPortalActor>(Actor);
		if (Portal)
		{
			if (Portal->GetClass()->GetName().Contains(TEXT("Boss")))
			{
				BossPortalActors.Add(Portal);
			}
			else
			{
				PFPortalActors.Add(Portal);
			}
		}
	}

	for (const FMonsterSpawnInfo& MonsterInfo : SpawnList)
	{
		TArray<APFPortalActor*>& TargetPortals = MonsterInfo.bIsBoss ? BossPortalActors : PFPortalActors;

		if (TargetPortals.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnManager: %s 좀비를 스폰해야 하는데 지정된 포탈이 맵에 없습니다!"), 
				MonsterInfo.bIsBoss ? TEXT("보스") : TEXT("일반"));
			continue;
		}

		for (int32 i = 0; i < MonsterInfo.SpawnCount; ++i)
		{
			int32 RandomIndex = FMath::RandRange(0, TargetPortals.Num() - 1);
			if (TargetPortals[RandomIndex])
			{
				TargetPortals[RandomIndex]->SpawnMonster(MonsterInfo.MonsterClass,MonsterInfo.bIsBoss);
			}
		}
	}
}


