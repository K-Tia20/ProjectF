

#include "Monster/PFMonsterCharacter.h"
#include  "Components/BoxComponent.h"

//최신2026-07-06
APFMonsterCharacter::APFMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
	AttackBox->SetupAttachment(GetRootComponent());
	
	AttackBox->SetBoxExtent(FVector(80.f,80.f,80.f));
	AttackBox->SetRelativeLocation(FVector(120.f, 0.f, 0.f));
	
	AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackBox->SetCollisionObjectType(ECC_WorldDynamic);
	AttackBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
}

void APFMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHP = MaxHP;
	
	AttackBox->OnComponentBeginOverlap.AddDynamic(this,&APFMonsterCharacter::OnAttackBoxOverlap);
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] BeginPlay / HP: %f"), CurrentHP);
	
}

void APFMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APFMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APFMonsterCharacter::TakeDamageFromMonster(float DamageAmount, AActor* DamageCauser)
{
	if (IsDead)
	{
		return;
	}
	
	CurrentHP -= DamageAmount;
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Hit! Damage: %f / HP: %f"),DamageAmount,CurrentHP);
	
	OnDamaged(DamageCauser);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}
}

void APFMonsterCharacter::Attack()
{
	if (IsDead)
	{
		return;
	}
	
	if (!CanAttack)
	{
		return;
	}
	
	CanAttack= false;
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Attack"));
	
	EnableAttackBox();
	
	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&APFMonsterCharacter::ResetAttack,
		AttackCooldown,
		false
		);
}

void APFMonsterCharacter::Die()
{
	if (IsDead)
	{
		return;
	}
	
	IsDead = true;
	CurrentHP = 0.f;
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] Die"));
}

void APFMonsterCharacter::ResetAttack()
{
	CanAttack = true;

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Ready"));
}

void APFMonsterCharacter::EnableAttackBox()
{
	AttackBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] AttackBox ON"));
	
	GetWorld()->GetTimerManager().SetTimer(
		AttackBoxTimerHandle,
		this,
		&APFMonsterCharacter::DisableAttackBox,
		0.2f,
		false
		);
}

void APFMonsterCharacter::DisableAttackBox()
{
	AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] AttackBox OFF"));
}

void APFMonsterCharacter::OnAttackBoxOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	if (!OtherActor||OtherActor ==this)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Hit: %s"), *OtherActor->GetName());
}

void APFMonsterCharacter::OnDamaged(AActor* DamageCauser)
{
	
}
