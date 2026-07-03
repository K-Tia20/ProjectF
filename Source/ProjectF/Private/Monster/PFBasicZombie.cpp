

#include "Monster/PFBasicZombie.h"

#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"


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
	
	if (Distance > DetectRange)
	{
		return;
	}
	
	TargetActor = Player;
	
	AAIController* AIController = Cast<AAIController>(GetController());
	
	if (!AIController)
	{
		UE_LOG(LogTemp,Warning,TEXT("[Zombie] AIController is NULL"));
		return;
	}
	
	if (Distance <= AttackRange)
	{
		AIController->StopMovement();
		
		//UE_LOG(LogTemp,Warning,TEXT("[Zombie] Attack Range"));
		
		Attack();
	}
	else
	{
		ChasePlayer();
		
		UE_LOG(LogTemp,Warning,TEXT("[Zombie] Chase Player"));
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