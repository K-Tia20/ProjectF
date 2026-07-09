// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFPortalActor.h"
#include "Monster/PFMonsterCharacter.h"



APFPortalActor::APFPortalActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 변수 생성 및 등록
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	RootComponent = PortalMesh;
}





void APFPortalActor :: SpawnMonster(TSubclassOf<APawn> MonsterClass, bool bIsBoss)
{
	if (MonsterClass == nullptr) return;

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (APFMonsterCharacter* SpawnedMonster = Cast<APFMonsterCharacter>(SpawnedPawn))
	{
		SpawnedMonster->bIsBoss = bIsBoss;
	}
	GetWorld()->SpawnActor<APawn>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);
}