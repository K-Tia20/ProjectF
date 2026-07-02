// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFAmmoItem.h"


// Sets default values
APFAmmoItem::APFAmmoItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APFAmmoItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APFAmmoItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

