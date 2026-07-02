// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFSpawnManager.h"


// Sets default values
APFSpawnManager::APFSpawnManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APFSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APFSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

