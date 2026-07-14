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
			if (Portal->ActorHasTag(TEXT("Boss")))
			{
				BossPortalActors.Add(Portal);
				UE_LOG(LogTemp, Log, TEXT("SpawnManager: 보스 포탈 발견! -> %s"), *Portal->GetName());
			}
			else
			{
				PFPortalActors.Add(Portal);
			}
	}

	for (AActor* Actor : FoundActors)
	{
		APFPortalActor* Portal = Cast<APFPortalActor>(Actor);
		if (Portal)
		{
			if (Portal->ActorHasTag(TEXT("Boss")))
			{
				BossPortalActors.Add(Portal);
				UE_LOG(LogTemp, Log, TEXT("SpawnManager: 보스 포탈 발견! -> %s"), *Portal->GetName());
			}
			else
			{
				PFPortalActors.Add(Portal);
			}
		}
	}

	for (const FMonsterSpawnInfo& MonsterInfo : SpawnList)
	{
		// 보스 여부에 따라 타겟 포탈 리스트 결정
		TArray<APFPortalActor*>& TargetPortals = MonsterInfo.bIsBoss ? BossPortalActors : PFPortalActors;

		if (TargetPortals.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnManager: %s 좀비를 스폰해야 하는데 지정된 포탈이 맵에 없습니다!"), 
			   MonsterInfo.bIsBoss ? TEXT("보스") : TEXT("일반"));
			continue;
		}

		// 설정된 태그와 일치하는 포탈 확인
		APFPortalActor* DesignatedPortal = nullptr;
		for (APFPortalActor* Portal : TargetPortals)
		{
			if (Portal && Portal->ActorHasTag(MonsterInfo.TargetPortalTag))
			{
				DesignatedPortal = Portal;
				break;
			}
		}

		if (DesignatedPortal == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnManager: [%s] 태그를 가진 포탈을 찾을 수 없습니다. 기본 포탈을 임시 지정합니다."), 
			   *MonsterInfo.TargetPortalTag.ToString());
			DesignatedPortal = TargetPortals[0];
		}
		
		for (int32 i = 0; i < MonsterInfo.SpawnCount; ++i)
		{
			DesignatedPortal->SpawnMonster(MonsterInfo.MonsterClass, MonsterInfo.bIsBoss);
		}
	}
}


