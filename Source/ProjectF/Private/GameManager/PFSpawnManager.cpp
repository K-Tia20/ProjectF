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





void APFSpawnManager::SpawnWaveMonsters(TSubclassOf<APawn> MonsterClass, int32 TotalAmount)
{
	if (PFPortalActors.Num() == 0 || MonsterClass == nullptr || TotalAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("스폰 조건이 올바르지 않습니다. (포탈 개수: %d)"), PFPortalActors.Num());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%d마리 스폰 시작!"), TotalAmount);

	for (int32 i = 0; i < TotalAmount; ++i)
	{
		int32 RandomPortalIndex = FMath::RandRange(0, PFPortalActors.Num() - 1);
		APFPortalActor* ChosenPortal = PFPortalActors[RandomPortalIndex];

		if (ChosenPortal)
		{
			ChosenPortal->SpawnMonster(MonsterClass);
		}
	}
}


