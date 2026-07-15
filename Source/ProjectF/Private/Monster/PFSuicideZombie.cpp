


#include "Monster/PFSuicideZombie.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"



APFSuicideZombie::APFSuicideZombie()
{

	PrimaryActorTick.bCanEverTick = true;
	
	// ===== Suicide Zombie Status =====
	MaxHP = 80.f;

	// 플레이어 가까이까지 접근
	AttackRange = 180.f;

	// 추적 속도
	WalkSpeed = 400.f;

	// 일반 공격은 사용하지 않지만 기본값은 남겨둔다.
	AttackDamage = 0.f;
}


void APFSuicideZombie::BeginPlay()
{
	Super::BeginPlay();
	
	bIsExploding = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] BeginPlay / Radius: %f / Damage: %f"),
		ExplosionRadius,
		ExplosionDamage
		);
	
}

void APFSuicideZombie::Attack()
{
	StartExplosion();
}

void APFSuicideZombie::StartExplosion()
{
	if (IsDead || bIsExploding)
	{
		return;
	}

	bIsExploding = true;
	bIsAttacking = true;
	
	// 자폭 준비 중에는 이동 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Explosion Started / Delay: %f"),
		ExplosionDelay
	);

	GetWorldTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&APFSuicideZombie::Explode,
		ExplosionDelay,
		false
	);
}

void APFSuicideZombie::Explode()
{
	if (IsDead)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Exploded / Radius: %f"),
		ExplosionRadius
	);
}


