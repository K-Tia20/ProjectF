// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFHealingItem.h"


// Sets default values
APFHealingItem::APFHealingItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APFHealingItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APFHealingItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

