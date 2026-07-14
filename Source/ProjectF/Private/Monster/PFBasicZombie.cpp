

#include "Monster/PFBasicZombie.h"

#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"


APFBasicZombie::APFBasicZombie()
{
	// 현재 좀비 AI는 Tick이 아니라 Timer 기반으로 감지 처리
	PrimaryActorTick.bCanEverTick = false;
}

void APFBasicZombie::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도 적용
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 최초 배치 위치 저장
	SpawnLocation = GetActorLocation();
	LastKnownLocation = SpawnLocation;

	// 0.5초마다 플레이어 감지
	GetWorldTimerManager().SetTimer(
		DetectTimerHandle,
		this,
		&APFBasicZombie::DetectPlayer,
		DetectInterval,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Basic Zombie BeginPlay"));
}


// =====================================================
// Detect
// =====================================================

void APFBasicZombie::DetectPlayer()
{
	if (IsDead)
	{
		return;
	}

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!Player)
	{
		return;
	}

	

	const float Distance = FVector::Dist(
		GetActorLocation(),
		Player->GetActorLocation()
	);

	// 디버그 시야 표시
	if (BDrawDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			DetectRange,
			32,
			FColor::Green,
			false,
			DetectInterval
		);

		DrawDebugCone(
			GetWorld(),
			GetActorLocation(),
			GetActorForwardVector(),
			DetectRange,
			FMath::DegreesToRadians(SightAngle * 0.5f),
			FMath::DegreesToRadians(SightAngle * 0.5f),
			16,
			FColor::Yellow,
			false,
			DetectInterval
		);
	}

	const bool BIsInDetectRange = Distance <= DetectRange;
	const bool BHasClearLine = BIsInDetectRange && HasLineOfSight(Player);
	const bool BIsInsideSightCone = IsPlayerInSight(Player);

	/*
	 * 처음 플레이어를 발견할 때는 시야각 안에 있어야 한다.
	 * 이미 타겟을 잡은 상태라면 시야각 밖으로 이동해도
	 * 장애물 없이 직접 보이는 동안에는 타겟을 유지한다.
	 */
	const bool BCanSeePlayer =
		BHasClearLine &&
		(BHasTarget || BIsInsideSightCone);


	// =====================================================
	// 플레이어가 보이는 경우
	// =====================================================

	if (BCanSeePlayer)
	{
		// 다시 봤으므로 시야 상실 시간 초기화
		CurrentLostSightTime = 0.f;
		
		// 처음 발견하는 경우에만 감지 시간을 누적한다.
		if (!BHasTarget)
		{
			CurrentDetectTime += DetectInterval;

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Zombie] Detecting Player: %f"),
				CurrentDetectTime
			);
			
			if (CurrentDetectTime < RequiredDetectTime)
			{
				return;
			}
		}
		else
		{
			// 이미 발견했던 타겟은 다시 감지 시간을 채우지 않는다.
			CurrentDetectTime = RequiredDetectTime;
		}
				
		const bool BWasHasTarget = BHasTarget;

		TargetActor = Player;
		LastKnownLocation = Player->GetActorLocation();

		BHasTarget = true;
		BMovingToLastKnownLocation = false;

		if (!BWasHasTarget)
		{
			PlayDetectSound();
		}

		// 다시 찾았으므로 마지막 위치 처리 중단
		GetWorldTimerManager().ClearTimer(
			LastKnownLocationTimerHandle
		);

		/*
		 * 공격 중에도 여기까지 실행되기 때문에
		 * LastKnownLocation과 TargetActor는 계속 갱신된다.
		 *
		 * 단, 공격 중에는 새로운 이동 명령을 내리지 않는다.
		 */
		if (bIsAttacking)
		{
			if (AAIController* AIController =
				Cast<AAIController>(GetController()))
			{
				AIController->StopMovement();
			}

			return;
		}

		// 공격 거리 안이면 공격
		if (Distance <= AttackRange)
		{
			if (AAIController* AIController =
				Cast<AAIController>(GetController()))
			{
				AIController->StopMovement();
			}

			Attack();
		}
		else
		{
			ChasePlayer();
		}

		return;
	}


	// =====================================================
	// 플레이어가 보이지 않는 경우
	// =====================================================

	CurrentDetectTime = 0.f;

	/*
	 * 공격 중에는 플레이어가 안 보여도
	 * 타겟을 잃거나 시야 상실 시간을 증가시키지 않는다.
	 *
	 * 따라서 긴 공격 애니메이션의 시간이
	 * LostSightGraceTime에 포함되지 않는다.
	 */
	if (bIsAttacking)
	{
		if (AAIController* AIController =
			Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}

		return;
	}

	// 원래 타겟이 없었다면 잃어버릴 타겟도 없다.
	if (!BHasTarget)
	{
		CurrentLostSightTime = 0.f;
		return;
	}

	/*
	 * MoveToActor 명령이 남아 있으면
	 * 벽 너머 플레이어를 계속 추적할 수 있으므로 중단한다.
	 */
	
	// 공격이 끝난 뒤부터 시야 상실 시간 누적
	CurrentLostSightTime += DetectInterval;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Zombie] Lost Sight: %f / %f"),
		CurrentLostSightTime,
		LostSightGraceTime
	);

	// 유예 시간 동안에는 TargetActor를 유지하고 계속 추적한다.
	// 더블 훅 중 플레이어가 시야 밖으로 빠져도 공격 종료 후 따라간다.
	if (CurrentLostSightTime < LostSightGraceTime)
	{
		ChasePlayer();
		return;
	}

	// 유예 시간이 끝났으므로 실제 타겟 추적을 중단한다.
	TargetActor = nullptr;
	BHasTarget = false;
	CurrentLostSightTime = 0.f;

	// 마지막으로 실제 확인했던 위치로 이동한다.
	HandleLostTarget();
}

bool APFBasicZombie::IsPlayerInSight(AActor* Player)
{
	if (!Player)
	{
		return false;
	}

	const FVector Forward = GetActorForwardVector();
	const FVector DirectionToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	const float Dot = FVector::DotProduct(Forward, DirectionToPlayer);

	const float HalfAngle = SightAngle * 0.5f;
	const float CosValue = FMath::Cos(FMath::DegreesToRadians(HalfAngle));

	return Dot >= CosValue;
}

bool APFBasicZombie::HasLineOfSight(AActor* Player)
{
	if (!Player)
	{
		return false;
	}

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector End = Player->GetActorLocation() + FVector(0.f, 0.f, 60.f);

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool BHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	const bool BCanSeePlayer = BHit && HitResult.GetActor() == Player;

	if (BDrawDebug)
	{
		DrawDebugLine(
			GetWorld(),
			Start,
			End,
			BCanSeePlayer ? FColor::Green : FColor::Red,
			false,
			0.5f,
			0,
			2.f
		);
	}

	return BCanSeePlayer;
}


// =====================================================
// Chase / Move
// =====================================================

void APFBasicZombie::ChasePlayer()
{
	if (bIsAttacking || !TargetActor)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());

	if (AIController)
	{
		AIController->MoveToActor( TargetActor,20.f,false );
	}
}

void APFBasicZombie::HandleLostTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Lost Target"));

	PlaySearchSound();
	
	MoveToLastKnownLocation();

	// 기존 타이머가 남아있을 수 있으므로 제거 후 다시 등록
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	GetWorldTimerManager().SetTimer(
		LastKnownLocationTimerHandle,
		this,
		&APFBasicZombie::CheckLastKnownLocation,
		0.3f,
		true
	);
}

void APFBasicZombie::MoveToLastKnownLocation()
{
	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	BMovingToLastKnownLocation = true;

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Move To Last Known Location"));

	AIController->MoveToLocation(
		LastKnownLocation,
		MoveAcceptanceRadius
	);
}

void APFBasicZombie::CheckLastKnownLocation()
{
	if (IsDead)
	{
		GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);
		return;
	}

	// 다시 플레이어를 찾은 상태면 마지막 위치 처리 중단
	if (TargetActor)
	{
		GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);
		BMovingToLastKnownLocation = false;
		return;
	}
	
	// 높이 차이는 제외하고 평면 거리만 확인
	const float DistanceToLastKnownLocation = FVector::Dist2D(
		GetActorLocation(),
		LastKnownLocation
	);

	// NavMesh 때문에 정확히 같은 위치에는 못 갈 수 있으므로 약간의 여유를 둔다.
	const float ReachDistance = MoveAcceptanceRadius + 50.f;
	
	// 아직 도착하지 않았으면 다음 타이머 검사까지 대기
	if (DistanceToLastKnownLocation > ReachDistance)
	{
		return;
	}
	
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}
		
	BMovingToLastKnownLocation = false;
	
	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Reached Last Known Location / Distance: %f"), DistanceToLastKnownLocation);

	HandleAfterReachedLastKnownLocation();
}

void APFBasicZombie::HandleAfterReachedLastKnownLocation()
{
	// 일반 좀비는 마지막 위치 확인 후 스폰 위치로 복귀
	// 보스는 이 함수를 override해서 수색 로직으로 바꿀 예정
	if (BReturnToSpawnLocation)
	{
		ReturnToSpawnLocation();
	}
}

void APFBasicZombie::ReturnToSpawnLocation()
{
	AAIController* AIController = Cast<AAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Return To Spawn"));

	AIController->MoveToLocation(
		SpawnLocation,
		MoveAcceptanceRadius
	);

	BMovingToLastKnownLocation = false;
}


// =====================================================
// Sound
// =====================================================

void APFBasicZombie::PlayRoarSound()
{
	if (!RoarSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		RoarSound,
		GetActorLocation()
	);
}

void APFBasicZombie::PlayDetectSound()
{
	if (!DetectSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		DetectSound,
		GetActorLocation()
	);
}

void APFBasicZombie::PlaySearchSound()
{
	if (!SearchSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		SearchSound,
		GetActorLocation()
	);
}

// =====================================================
// Damage
// =====================================================

void APFBasicZombie::OnDamaged(AActor* DamageCauser)
{
	if (IsDead)
	{
		return;
	}

	// DamageCauser가 없으면 일단 플레이어를 공격자로 간주
	if (!DamageCauser)
	{
		DamageCauser = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (!DamageCauser)
	{
		return;
	}

	TargetActor = DamageCauser;
	CurrentDetectTime = RequiredDetectTime;
	CurrentLostSightTime = 0.f;
	LastKnownLocation = DamageCauser->GetActorLocation();

	BHasTarget = true;
	BMovingToLastKnownLocation = false;

	// 피격으로 타겟을 잡았으므로 마지막 위치 체크 타이머 제거
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detected attacker immediately"));

	ChasePlayer();
}


// =====================================================
// Dead
// =====================================================

void APFBasicZombie::Die()
{
	GetWorldTimerManager().ClearTimer(DetectTimerHandle);
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	TargetActor = nullptr;
	BHasTarget = false;
	BMovingToLastKnownLocation = false;
	
	CurrentDetectTime = 0.f;
	CurrentLostSightTime = 0.f;

	Super::Die();
}