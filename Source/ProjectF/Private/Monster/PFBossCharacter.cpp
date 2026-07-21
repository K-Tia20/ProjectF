


#include "Monster/PFBossCharacter.h"

#include "Monster/PFMonsterAIController.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationSystem.h"
#include "Monster/PFBasicZombie.h"



APFBossCharacter::APFBossCharacter()
{
	bIsBoss = true;
	
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
	WalkSpeed = 850.f;
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

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// 최고속도 850
		Movement->MaxWalkSpeed = WalkSpeed;

		/*
		 * 1초에 약 450씩 속도가 증가한다.
		 * 
		 */
		Movement->MaxAcceleration = 450.f;

		// 정지할 때의 감속도
		Movement->BrakingDecelerationWalking = 600.f;

		/*
		 * AI의 MoveTo도 즉시 최고속도로 이동하지 않고
		 * CharacterMovement의 가속도를 사용하게 한다.
		 */
		if (FNavMovementProperties* NavProperties = Movement->GetNavMovementProperties())
		{
			NavProperties->bUseAccelerationForPaths = true;
			
			// 목표 지점 10cm 전부터만 감속
			NavProperties->bUseFixedBrakingDistanceForPaths = true;
			NavProperties->FixedPathBrakingDistance = 10.f;
		}
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
	
	const bool BWasArmorBroken = IsArmorBroken();

	CurrentArmor = FMath::Clamp(CurrentArmor - ArmorDamageAmount, 0.f, MaxArmor);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Armor Damaged / ArmorDamage: %f / CurrentArmor: %f"),
		ArmorDamageAmount,
		CurrentArmor
	);

	if (!BWasArmorBroken && IsArmorBroken())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] Armor Broken"));
		
		if (ArmorBreakSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(),
				ArmorBreakSound,
				GetActorLocation()
			);
		}
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
	
	if (Phase2Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Phase2Sound,
			GetActorLocation()
		);
	}

	// 2페이즈 강화
	WalkSpeed += 150.f;
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
	bIsAttacking = true;
	CanAttack = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Roar Summon Pattern Start")
	);

	// AI 이동 정지
	if (AController* BossController = GetController())
	{
		BossController->StopMovement();
	}

	// 캐릭터 이동 정지
	if (UCharacterMovementComponent* Movement =
		GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 포효 공통 사운드
	PlayRoarSound();

	// 포효 소환 몽타주 재생
	if (RoarSummonMontage)
	{
		const float MontageDuration =
			PlayAnimMontage(RoarSummonMontage);

		if (MontageDuration > 0.f)
		{
			return;
		}
	}

	// 몽타주가 없거나 재생 실패한 경우 바로 소환
	HandleRoarSummonNotify();
}

void APFBossCharacter::SummonMinions()
{
	if (IsDead || !SummonMonsterClass || !GetWorld())
	{
		return;
	}

	UNavigationSystemV1* NavSystem =
		UNavigationSystemV1::GetCurrent(GetWorld());

	if (!NavSystem)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Boss] Navigation System Not Found")
		);

		return;
	}

	int32 SpawnedCount = 0;

	// 일부 위치에서 소환에 실패할 수 있으므로
	// 소환 개수보다 더 많이 위치를 시도한다.
	const int32 MaxSpawnAttempts = SummonCount * 5;

	for (int32 Attempt = 0;
		 Attempt < MaxSpawnAttempts &&
		 SpawnedCount < SummonCount;
		 ++Attempt)
	{
		FNavLocation RandomNavLocation;

		const bool bFoundLocation =
			NavSystem->GetRandomReachablePointInRadius(
				GetActorLocation(),
				SummonRadius,
				RandomNavLocation
			);

		if (!bFoundLocation)
		{
			continue;
		}

		FVector MinionSpawnLocation = RandomNavLocation.Location;

		// 땅에 너무 박히는 것을 방지
		MinionSpawnLocation.Z += 20.f;

		// 보스 쪽을 바라보도록 회전
		const FRotator SpawnRotation =
			(GetActorLocation() - MinionSpawnLocation).Rotation();

		FActorSpawnParameters SpawnParams;

		SpawnParams.Owner = this;

		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::
			AdjustIfPossibleButDontSpawnIfColliding;

		APFBasicZombie* SpawnedMonster =
			GetWorld()->SpawnActor<APFBasicZombie>(
				SummonMonsterClass,
				MinionSpawnLocation,
				SpawnRotation,
				SpawnParams
			);

		if (!SpawnedMonster)
		{
			continue;
		}

		++SpawnedCount;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Boss] Minion Spawned: %s / %d of %d"),
			*SpawnedMonster->GetName(),
			SpawnedCount,
			SummonCount
		);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Summon Finished / Spawned: %d of %d"),
		SpawnedCount,
		SummonCount
	);
}

void APFBossCharacter::EndPattern()
{
	BIsUsingPattern = false;
	bIsAttacking = false;
	CanAttack = true;

	if (!IsDead)
	{
		if (UCharacterMovementComponent* Movement =
			GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Pattern End")
	);
	
}

void APFBossCharacter::ResetRoarSummonCooldown()
{
	if (IsDead)
	{
		return;
	}

	bCanRoarSummon = true;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Roar Summon Ready")
	);
}

void APFBossCharacter::HandleRoarSummonNotify()
{
	// 이미 패턴이 끝났거나 죽었다면 중복 소환 방지
	if (IsDead || !BIsUsingPattern)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Boss] Roar Summon Notify")
	);

	// 자폭몹이 나타나는 순간 소환 사운드 재생
	if (SummonSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			SummonSound,
			GetActorLocation()
		);
	}

	SummonMinions();
	EndPattern();
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

// =====================================================
// Attack pattern
// =====================================================

void APFBossCharacter::Attack()
{
	if (IsDead || BIsUsingPattern)
	{
		return;
	}

	// 소환할 클래스가 지정되어 있고 쿨타임이 준비됐으면 소환
	if (bCanRoarSummon && SummonMonsterClass)
	{
		bCanRoarSummon = false;

		StartRoarSummonPattern();

		GetWorldTimerManager().SetTimer(
			RoarSummonTimerHandle,
			this,
			&APFBossCharacter::ResetRoarSummonCooldown,
			RoarSummonCooldown,
			false
		);

		return;
	}

	// 소환 쿨타임 중에는 기존 주먹 공격
	Super::Attack();
}
