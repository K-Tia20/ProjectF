// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/PFGrenade.h"


// Sets default values
APFGrenade::APFGrenade()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APFGrenade::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APFGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

