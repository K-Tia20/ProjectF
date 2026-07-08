

#include "Monster/PFBasicZombie.h"

#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"


APFBasicZombie::APFBasicZombie()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APFBasicZombie::BeginPlay()
{
	Super::BeginPlay();
			
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	SpawnLocation = GetActorLocation();
	LastKnownLocation = SpawnLocation;
	
	GetWorldTimerManager().SetTimer(DetectTimerHandle,this,&APFBasicZombie::DetectPlayer,0.5f,true);
	
	UE_LOG(LogTemp,Warning,TEXT("[Zombie] Basic Zombie BeginPlay"));
	
}

void APFBasicZombie::DetectPlayer()
{
	if (IsDead)
	{
		return;
	}
	
	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	
	if (!Player)
	{
		return;
	}
	
	float Distance = FVector::Dist(GetActorLocation(),Player->GetActorLocation());
	
	//시야 체크 코딩
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
	
	bool BCanSeePlayer =
	Distance <= DetectRange &&
	IsPlayerInSight(Player) &&
	HasLineOfSight(Player);
	
	if (!BCanSeePlayer)
	{
		CurrentDetectTime = 0.f;
		
		if (BHasTarget)
		{
			TargetActor = nullptr;
			BHasTarget = false;

			MoveToLastKnownLocation();
			
			GetWorldTimerManager().SetTimer(
			LastKnownLocationTimerHandle,
			this,
			&APFBasicZombie::CheckLastKnownLocation,
			2.0f,
			false
			);
		}

		return;
	}
	
	CurrentDetectTime += 0.5f;
	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detecting Player: %f"), CurrentDetectTime);
	
	if (CurrentDetectTime < RequiredDetectTime)
	{
		return;
	}

	TargetActor = Player;
	
	LastKnownLocation = Player->GetActorLocation();
	
	BHasTarget = true;
	BMovingToLastKnownLocation = false;
				
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
		
		//UE_LOG(LogTemp,Warning,TEXT("[Zombie] Chase Player"));
	}
			
}

void APFBasicZombie::ChasePlayer()
{
	if (!TargetActor)
	{
		return;
	}
	
	AAIController* AIController = Cast<AAIController>(GetController());
	
	if (AIController)
	{
		AIController->MoveToActor(TargetActor,MoveAcceptanceRadius);
	}
}
//시야각
bool APFBasicZombie::IsPlayerInSight(AActor* Player)
{
	if (!Player)
	{
		return false;
	}

	FVector Forward = GetActorForwardVector();
	FVector DirectionToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	float Dot = FVector::DotProduct(Forward, DirectionToPlayer);

	float HalfAngle = SightAngle * 0.5f;
	float CosValue = FMath::Cos(FMath::DegreesToRadians(HalfAngle));

	return Dot >= CosValue;
}

void APFBasicZombie::OnDamaged(AActor* DamageCauser)
{
	if (IsDead)
	{
		return;
	}

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

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detected attacker immediately"));

	ChasePlayer();
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

	AIController->MoveToLocation(LastKnownLocation, MoveAcceptanceRadius);
}

void APFBasicZombie::CheckLastKnownLocation()
{
	if (TargetActor)
	{
		return;
	}

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
//시야 벽막힘 구현 및 디버그
bool APFBasicZombie::HasLineOfSight(AActor* Player)
{
	if (!Player)
	{
		return false;
	}

	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	FVector End = Player->GetActorLocation() + FVector(0.f, 0.f, 60.f);

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool BHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	bool BCanSeePlayer = BHit && HitResult.GetActor() == Player;

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
//Dead
void APFBasicZombie::Die()
{
	GetWorldTimerManager().ClearTimer(DetectTimerHandle);
	GetWorldTimerManager().ClearTimer(LastKnownLocationTimerHandle);

	TargetActor = nullptr;
	BHasTarget = false;
	BMovingToLastKnownLocation = false;

	Super::Die();
}