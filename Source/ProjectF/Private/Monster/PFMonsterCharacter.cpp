

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
	
	if (!CanAttack)
	{
		return;
	}
	
	CanAttack= false;
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Attack"));
	
	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&APFMonsterCharacter::ResetAttack,
		AttackCooldown,
		false
		);
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

void APFMonsterCharacter::ResetAttack()
{
	CanAttack = true;

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Ready"));
}

void APFMonsterCharacter::OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//함수 구현부 빌드 껍데기용 추후에 지워도됨
}
