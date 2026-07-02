// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/PFBossCharacter.h"


// Sets default values
APFBossCharacter::APFBossCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APFBossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APFBossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APFBossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

