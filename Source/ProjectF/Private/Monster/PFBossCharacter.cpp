

#include "Monster/PFBossCharacter.h"


APFBossCharacter::APFBossCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APFBossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APFBossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APFBossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

