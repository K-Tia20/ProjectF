

#include "Monster/PFMonsterCharacter.h"



APFMonsterCharacter::APFMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APFMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;

	UE_LOG(LogTemp, Warning, TEXT("[Monster] BeginPlay / HP: %f"), CurrentHP);
	
}

void APFMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APFMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APFMonsterCharacter::TakeDamageFromMonster(float DamageAmount)
{
	if (IsDead)
	{
		return;
	}
	
	CurrentHP -= DamageAmount;
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Hit! Damage: %f / HP: %f"),DamageAmount,CurrentHP);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}
}

void APFMonsterCharacter::Attack()
{
	if (IsDead)
	{
		return;
	}
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Attack"));
}

void APFMonsterCharacter::Die()
{
	if (IsDead)
	{
		return;
	}
	
	IsDead = true;
	CurrentHP = 0.f;
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] Die"));
}
