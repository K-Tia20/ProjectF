

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
		0.5f,
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
	
	// 공격 애니메이션 중에는 감지 및 추적 명령을 새로 내리지 않는다.
	if (bIsAttacking)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}

		return;
	}

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!Player)
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

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
			0.5f
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
			0.5f
		);
	}

	const bool BCanSeePlayer =
		Distance <= DetectRange &&
		IsPlayerInSight(Player) &&
		HasLineOfSight(Player);

	// 플레이어를 볼 수 없는 경우
	if (!BCanSeePlayer)
	{
		CurrentDetectTime = 0.f;

		// 이전에 타겟을 가지고 있었다면, 플레이어를 놓친 상태
		if (BHasTarget)
		{
			TargetActor = nullptr;
			BHasTarget = false;

			HandleLostTarget();
		}

		return;
	}

	// 플레이어를 보고 있는 시간 누적
	CurrentDetectTime += 0.5f;

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detecting Player: %f"), CurrentDetectTime);

	// 일정 시간 이상 봐야 정식 타겟으로 인식
	if (CurrentDetectTime < RequiredDetectTime)
	{
		return;
	}
	
	const bool BWasHasTarget = BHasTarget;

	// 플레이어 추적 시작
	TargetActor = Player;
	LastKnownLocation = Player->GetActorLocation();

	BHasTarget = true;
	BMovingToLastKnownLocation = false;
	
	if (!BWasHasTarget)
	{
		PlayDetectSound();
	}

	// 다시 플레이어를 찾았으므로 마지막 위치 체크 타이머 제거
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	// 공격 거리 안이면 공격
	if (Distance <= AttackRange)
	{
		AAIController* AIController = Cast<AAIController>(GetController());

		if (AIController)
		{
			AIController->StopMovement();
		}

		Attack();
	}
	else
	{
		ChasePlayer();
	}
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
		AIController->MoveToActor(TargetActor, MoveAcceptanceRadius);
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
		2.0f,
		false
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
		return;
	}

	// 다시 플레이어를 찾은 상태면 마지막 위치 처리 중단
	if (TargetActor)
	{
		return;
	}

	BMovingToLastKnownLocation = false;

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

	Super::Die();
}