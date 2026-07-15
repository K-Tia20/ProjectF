


#include "Monster/PFSuicideZombie.h"

#include "Monster/PFMonsterCharacter.h"
#include "Monster/PFBossCharacter.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "AIController.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"
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
	
	// 폭발 범위 디버그 표시
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		ExplosionRadius,
		32,
		FColor::Red,
		false,
		2.f,
		0,
		5.f
		);
	
	// 플레이어와 몬스터는 Character/Pawn이므로 Pawn 채널 탐색
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	ObjectTypes.Add(
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	);

	// 자기 자신은 범위 탐색에서 제외
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<AActor*> OverlappedActors;

	const bool bFoundActors =
		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			GetActorLocation(),
			ExplosionRadius,
			ObjectTypes,
			AActor::StaticClass(),
			ActorsToIgnore,
			OverlappedActors
		);

	if (bFoundActors)
	{
		/*
		 * 몬스터가 폭발 피해를 받았을 때
		 * 곧 죽을 자폭 몬스터를 추적하지 않도록
		 * 플레이어를 DamageCauser로 전달한다.
		 */
		AActor* MonsterDamageCauser =
			UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (!MonsterDamageCauser)
		{
			MonsterDamageCauser = this;
		}

		for (AActor* HitActor : OverlappedActors)
		{
			if (!IsValid(HitActor) || HitActor == this)
			{
				continue;
			}

			// =================================================
			// 보스 몬스터
			// 체력 피해와 방어도 피해를 모두 적용
			// =================================================

			if (APFBossCharacter* Boss =
				Cast<APFBossCharacter>(HitActor))
			{
				/*
				 * 현재 방어도가 이번 폭발 피해를 먼저 감소시키고,
				 * 그다음 방어도가 깎이도록 처리한다.
				 */
				Boss->TakeDamageFromMonster(
					ExplosionDamage,
					MonsterDamageCauser
				);

				Boss->TakeArmorDamage(ArmorDamage);

				UE_LOG(
					LogTemp,
					Warning,
					TEXT(
						"[SuicideZombie] Boss Hit / Damage: %f / ArmorDamage: %f"
					),
					ExplosionDamage,
					ArmorDamage
				);

				continue;
			}

			// =================================================
			// 일반 몬스터 및 다른 자폭 몬스터
			// 피아 구분 없이 체력 피해 적용
			// =================================================

			if (APFMonsterCharacter* Monster =
				Cast<APFMonsterCharacter>(HitActor))
			{
				Monster->TakeDamageFromMonster(
					ExplosionDamage,
					MonsterDamageCauser
				);

				UE_LOG(
					LogTemp,
					Warning,
					TEXT(
						"[SuicideZombie] Monster Hit: %s / Damage: %f"
					),
					*Monster->GetName(),
					ExplosionDamage
				);

				continue;
			}

			// =================================================
			// 플레이어 또는 일반 데미지 처리 액터
			// =================================================

			UGameplayStatics::ApplyDamage(
				HitActor,
				ExplosionDamage,
				GetController(),
				this,
				UDamageType::StaticClass()
			);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"[SuicideZombie] Actor Hit: %s / Damage: %f"
				),
				*HitActor->GetName(),
				ExplosionDamage
			);
		}
	}

	// 폭발 처리가 모두 끝난 뒤 자폭 몬스터 사망
	Die();
}


