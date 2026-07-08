


#include "Monster/PFBossCharacter.h"

#include "Monster/PFMonsterAIController.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"


APFBossCharacter::APFBossCharacter()
{
	// 보스는 스폰 위치로 돌아가지 않는다.
	BReturnToSpawnLocation = false;

	// ===== Boss Status =====
	MaxHP = 800.f;
	CurrentHP = MaxHP;

	AttackDamage = 50.f;
	AttackRange = 220.f;
	AttackCooldown = 1.8f;

	// ===== Boss Detect =====
	DetectRange = 2500.f;
	SightAngle = 120.f;
	RequiredDetectTime = 0.3f;

	// ===== Boss Move =====
	WalkSpeed = 300.f;
	MoveAcceptanceRadius = 80.f;

	// ===== Boss Armor =====
	MaxArmor = 100.f;
	CurrentArmor = MaxArmor;
	ArmorDamageReductionRate = 0.7f;

	// ===== Boss Search =====
	SearchRadius = 800.f;
	SearchInterval = 2.0f;
	BInfiniteSearch = true;
	MaxSearchCount = 5;
	CurrentSearchCount = 0;
	BIsSearching = false;

	// ===== Boss Phase =====
	BossPhase = 1;
	Phase2HPRatio = 0.5f;
	BIsPhase2 = false;

	// ===== Boss Pattern =====
	BIsUsingPattern = false;
	RoarSummonCooldown = 10.f;
	SummonCount = 3;
	SummonRadius = 600.f;
}

void APFBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	// BP에서 값이 바뀌었을 수 있으니 BeginPlay에서 다시 초기화
	BReturnToSpawnLocation = false;

	CurrentHP = MaxHP;
	CurrentArmor = MaxArmor;

	BossPhase = 1;
	BIsPhase2 = false;

	BIsSearching = false;
	CurrentSearchCount = 0;

	BIsUsingPattern = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Boss] BeginPlay / HP: %f / Armor: %f"), CurrentHP, CurrentArmor);
}


// =====================================================
// Damage / Armor
// =====================================================

void APFBossCharacter::TakeDamageFromMonster(float DamageAmount, AActor* DamageCauser)
{
	if (IsDead)
	{
		return;
	}

	float FinalDamage = DamageAmount;

	// 방어도가 남아있으면 데미지 감소
	if (CurrentArmor > 0.f)
	{
		const float ClampedReductionRate = FMath::Clamp(ArmorDamageReductionRate, 0.f, 0.95f);
		FinalDamage = DamageAmount * (1.f - ClampedReductionRate);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Hit / OriginalDamage: %f / FinalDamage: %f / Armor: %f"),
		DamageAmount,
		FinalDamage,
		CurrentArmor
	);

	Super::TakeDamageFromMonster(FinalDamage, DamageCauser);

	UpdateBossPhase();
}

void APFBossCharacter::TakeArmorDamage(float ArmorDamageAmount)
{
	if (IsDead)
	{
		return;
	}

	if (ArmorDamageAmount <= 0.f)
	{
		return;
	}

	CurrentArmor = FMath::Clamp(CurrentArmor - ArmorDamageAmount, 0.f, MaxArmor);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Armor Damaged / ArmorDamage: %f / CurrentArmor: %f"),
		ArmorDamageAmount,
		CurrentArmor
	);

	if (IsArmorBroken())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] Armor Broken"));
	}
}

bool APFBossCharacter::IsArmorBroken() const
{
	return CurrentArmor <= 0.f;
}

void APFBossCharacter::OnDamaged(AActor* DamageCauser)
{
	// APFBasicZombie의 피격 시 즉시 추적 로직 사용
	Super::OnDamaged(DamageCauser);

	// 맞았으면 수색 중단
	StopSearch();

	UpdateBossPhase();

	UE_LOG(LogTemp, Warning, TEXT("[Boss] OnDamaged"));
}


// =====================================================
// Search
// =====================================================

void APFBossCharacter::HandleAfterReachedLastKnownLocation()
{
	// 일반 좀비는 여기서 SpawnLocation으로 돌아가지만,
	// 보스는 마지막으로 본 위치 주변을 계속 수색한다.
	StartSearchAroundLastKnownLocation();
}

void APFBossCharacter::StartSearchAroundLastKnownLocation()
{
	if (IsDead)
	{
		return;
	}

	if (TargetActor)
	{
		return;
	}

	BIsSearching = true;
	CurrentSearchCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("[Boss] Start Search Around LastKnownLocation"));

	MoveToRandomSearchLocation();

	GetWorldTimerManager().ClearTimer(SearchTimerHandle);

	GetWorldTimerManager().SetTimer(
		SearchTimerHandle,
		this,
		&APFBossCharacter::MoveToRandomSearchLocation,
		SearchInterval,
		true
	);
}

void APFBossCharacter::MoveToRandomSearchLocation()
{
	if (IsDead)
	{
		StopSearch();
		return;
	}

	// 수색 중 플레이어를 다시 찾았으면 수색 중단
	if (TargetActor)
	{
		StopSearch();
		return;
	}

	if (!BInfiniteSearch && CurrentSearchCount >= MaxSearchCount)
	{
		StopSearch();
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] NavSystem is null"));
		return;
	}

	FNavLocation RandomLocation;

	const bool BFoundLocation = NavSystem->GetRandomReachablePointInRadius(
		LastKnownLocation,
		SearchRadius,
		RandomLocation
	);

	if (!BFoundLocation)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] Failed To Find Search Location"));
		return;
	}

	CurrentSearchLocation = RandomLocation.Location;

	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	AIController->MoveToLocation(
		CurrentSearchLocation,
		MoveAcceptanceRadius
	);

	CurrentSearchCount++;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Move To Search Location / Count: %d / Location: %s"),
		CurrentSearchCount,
		*CurrentSearchLocation.ToString()
	);
}

void APFBossCharacter::StopSearch()
{
	if (!BIsSearching)
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
		return;
	}

	BIsSearching = false;
	CurrentSearchCount = 0;

	GetWorldTimerManager().ClearTimer(SearchTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("[Boss] Stop Search"));
}


// =====================================================
// Phase
// =====================================================

void APFBossCharacter::UpdateBossPhase()
{
	if (IsDead)
	{
		return;
	}

	if (BIsPhase2)
	{
		return;
	}

	if (MaxHP <= 0.f)
	{
		return;
	}

	const float HPRatio = CurrentHP / MaxHP;

	if (HPRatio > Phase2HPRatio)
	{
		return;
	}

	BIsPhase2 = true;
	BossPhase = 2;

	// 2페이즈 강화
	WalkSpeed += 80.f;
	AttackCooldown = FMath::Max(0.8f, AttackCooldown * 0.7f);
	SearchInterval = FMath::Max(1.0f, SearchInterval * 0.7f);
	RoarSummonCooldown = FMath::Max(5.0f, RoarSummonCooldown * 0.7f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Phase 2 Start / WalkSpeed: %f / AttackCooldown: %f"),
		WalkSpeed,
		AttackCooldown
	);
}


// =====================================================
// Pattern - Summon
// 지금은 자리만 잡아둠. 실제 소환은 다음 단계에서 구현.
// =====================================================

void APFBossCharacter::StartRoarSummonPattern()
{
	if (IsDead || BIsUsingPattern)
	{
		return;
	}

	BIsUsingPattern = true;

	UE_LOG(LogTemp, Warning, TEXT("[Boss] Roar Summon Pattern Start"));

	// 나중에 여기서 포효 애니메이션 Montage 재생
	// 지금은 바로 소환 함수 호출 가능
	SummonMinions();

	EndPattern();
}

void APFBossCharacter::SummonMinions()
{
	// 다음 단계에서 구현 예정
	// 목표:
	// 1. SummonMonsterClass가 있으면
	// 2. 보스 주변 또는 지정된 스폰 지점에 부하 생성
	// 3. 생성된 부하는 보스 위치로 이동
	// 4. 이동 중 플레이어 발견 시 플레이어 추적

	UE_LOG(LogTemp, Warning, TEXT("[Boss] SummonMinions - Not Implemented Yet"));
}

void APFBossCharacter::EndPattern()
{
	BIsUsingPattern = false;

	UE_LOG(LogTemp, Warning, TEXT("[Boss] Pattern End"));
}


// =====================================================
// Pattern - Future
// =====================================================

void APFBossCharacter::StartChargePattern()
{
	// 나중에 돌진 패턴 구현 예정
	UE_LOG(LogTemp, Warning, TEXT("[Boss] Charge Pattern - Not Implemented Yet"));
}

void APFBossCharacter::StartRangedAttackPattern()
{
	// 나중에 원거리 패턴 구현 예정
	UE_LOG(LogTemp, Warning, TEXT("[Boss] Ranged Attack Pattern - Not Implemented Yet"));
}


// =====================================================
// Dead
// =====================================================

void APFBossCharacter::Die()
{
	GetWorldTimerManager().ClearTimer(SearchTimerHandle);
	GetWorldTimerManager().ClearTimer(RoarSummonTimerHandle);

	BIsSearching = false;
	BIsUsingPattern = false;

	UE_LOG(LogTemp, Warning, TEXT("[Boss] Die"));

	Super::Die();
}