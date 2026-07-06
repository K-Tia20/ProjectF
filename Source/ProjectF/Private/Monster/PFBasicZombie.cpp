

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
	if (bDrawDebug)
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
	if (Distance > DetectRange || !IsPlayerInSight(Player))
	{
		CurrentDetectTime = 0.f;
		return;
	}
	
	CurrentDetectTime += 0.5f;
	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detecting Player: %f"), CurrentDetectTime);
	
	if (CurrentDetectTime < RequiredDetectTime)
	{
		return;
	}

	TargetActor = Player;
				
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

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Detected attacker immediately"));

	ChasePlayer();
}

