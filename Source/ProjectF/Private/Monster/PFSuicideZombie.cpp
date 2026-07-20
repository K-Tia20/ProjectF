


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
#include "Components/BoxComponent.h"


APFSuicideZombie::APFSuicideZombie()
{

	PrimaryActorTick.bCanEverTick = false;
	
	// ===== Suicide Zombie Status =====
	MaxHP = 80.f;

	// 플레이어 가까이까지 접근
	AttackRange = 180.f;

	// 추적 속도
	WalkSpeed = 400.f;

	// 일반 공격은 사용하지 않지만 기본값은 남겨둔다.
	AttackDamage = 0.f;
	
	// =====================================================
	// Weak Point - Abdomen
	// =====================================================

	WeakPointCollision =
		CreateDefaultSubobject<UBoxComponent>(TEXT("WeakPointCollision"));

	WeakPointCollision->SetupAttachment(
		GetMesh(),
		TEXT("WeakPointSocket")
	);

	WeakPointCollision->SetBoxExtent(FVector(20.f, 20.f, 20.f));

	WeakPointCollision->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	WeakPointCollision->SetCollisionObjectType(ECC_WorldDynamic);
	WeakPointCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeakPointCollision->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block
	);


	// =====================================================
	// Weak Point - Chest
	// =====================================================

	WeakPointCollision2 =
		CreateDefaultSubobject<UBoxComponent>(TEXT("WeakPointCollision2"));

	WeakPointCollision2->SetupAttachment(
		GetMesh(),
		TEXT("WeakPointSocket2")
	);

	WeakPointCollision2->SetBoxExtent(FVector(20.f, 20.f, 20.f));

	WeakPointCollision2->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	WeakPointCollision2->SetCollisionObjectType(ECC_WorldDynamic);
	WeakPointCollision2->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeakPointCollision2->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block
	);
}


void APFSuicideZombie::BeginPlay()
{
	Super::BeginPlay();
	
	bIsExploding = false;
	bHasExploded = false;
	bIsCorpseBomb = false;

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
	if (IsDead || bIsExploding || bHasExploded)
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
	
	// 자폭 준비 몽타주 재생
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
	
	// 자폭 준비 시간 동안 폭발 범위 표시
	if (bDrawExplosionRange)
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			ExplosionRadius,
			32,
			FColor::Orange,
			false,
			ExplosionDelay,
			0,
			4.f
		);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Explosion Started / Delay: %f"),
		ExplosionDelay
	);
	
	// 디버그 표시를 꺼도 폭발 타이머는 항상 실행
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
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;
	bIsExploding = false;
	bIsCorpseBomb = false;
	bIsAttacking = false;

	GetWorldTimerManager().ClearTimer(ExplosionTimerHandle);
	
	// 살아 있는 상태는 액터 위치,
	// 래그돌 시체 상태는 현재 메시 위치를 폭발 중심으로 사용
	FVector ExplosionLocation = GetActorLocation();

	if (IsDead && GetMesh())
	{
		ExplosionLocation = GetMesh()->GetComponentLocation();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Exploded / Radius: %f"),
		ExplosionRadius
		);
	
	// 폭발 범위 디버그 표시
	if (bDrawExplosionRange)
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionLocation,
			ExplosionRadius,
			32,
			FColor::Red,
			false,
			2.f,
			0,
			5.f
			);
	}
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
			ExplosionLocation,
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

	// 살아 있는 상태에서 자폭했다면 일반 사망 처리
	if (!IsDead)
	{
		Super::Die();
	}

	// 이미 죽은 시체 폭탄이든 살아 있던 자폭이든
	// 폭발 후 잠시 뒤 제거
	SetLifeSpan(0.2f);
}

void APFSuicideZombie::Die()
{
	if (bHasExploded)
	{
		return;
	}

	// 이미 자폭 준비를 시작한 상태에서 죽으면 즉시 폭발
	if (bIsExploding)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[SuicideZombie] Killed During Explosion Charge")
		);

		Explode();
		return;
	}

	// 준비 전 죽었다면 우선 래그돌 사망
	Super::Die();

	// 부모 Die에서 설정한 자동 제거 시간을 취소
	SetLifeSpan(0.f);

	bIsCorpseBomb = true;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Corpse Bomb Armed")
	);
}

void APFSuicideZombie::TriggerWeakPointExplosion()
{
	if (bHasExploded)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SuicideZombie] Weak Point Hit")
	);

	Explode();
}

bool APFSuicideZombie::IsWeakPointComponent(
	const UPrimitiveComponent* HitComponent
) const
{
	return HitComponent == WeakPointCollision ||
		   HitComponent == WeakPointCollision2;
}
