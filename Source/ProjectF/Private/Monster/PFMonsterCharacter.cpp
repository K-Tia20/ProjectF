

#include "Monster/PFMonsterCharacter.h"
#include  "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include  "GameManager/PFGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

//최신2026-07-10
APFMonsterCharacter::APFMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
	AttackBox->SetupAttachment(GetMesh(), TEXT("AttackSocket_R"));

	AttackBox->SetBoxExtent(FVector(30.f, 30.f, 30.f));
	AttackBox->SetRelativeLocation(FVector::ZeroVector);
	AttackBox->SetRelativeRotation(FRotator::ZeroRotator);
	
	AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackBox->SetCollisionObjectType(ECC_WorldDynamic);
	AttackBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
}

void APFMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AttackBox && GetMesh())
	{
		AttackBox->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("AttackSocket_R")
		);

		AttackBox->SetRelativeLocation(FVector::ZeroVector);
		AttackBox->SetRelativeRotation(FRotator::ZeroRotator);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Monster] AttackBox Parent: %s / Socket: %s"),
			*GetNameSafe(AttackBox->GetAttachParent()),
			*AttackBox->GetAttachSocketName().ToString()
		);
	}
	
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
	//사운드
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			HitSound,
			GetActorLocation()
		);
	}
	
	CurrentHP -= DamageAmount;
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Hit! Damage: %f / HP: %f"),DamageAmount,CurrentHP);
	
	// UI 갱신용 블루프린트 이벤트 호출
	OnMonsterDamaged(DamageAmount, DamageCauser);
	
	OnDamaged(DamageCauser);
	
	if (CurrentHP <= 0.f)
	{
		Die();
	}
}

void APFMonsterCharacter::Attack()
{
	if (IsDead || !CanAttack || bIsAttacking)

	{
		return;
	}
	
	CanAttack= false;
	bIsAttacking = true;
	
	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	
	UE_LOG(LogTemp,Warning,TEXT("[Monster] Attack"));
	
	if (AttackMontage && GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
		}
	}
	
	//사운드
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			AttackSound,
			GetActorLocation()
		);
	}
		
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
	bIsAttacking = false;
	CanAttack = false;

	HitActorsThisAttack.Empty();

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Die - Ragdoll"));

	// 공격 관련 타이머 및 판정 정리
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackBoxTimerHandle);

	DisableAttackBox();

	// AI 이동 중지
	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
	}

	// 캐릭터 이동 중지
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 재생 중인 공격 몽타주 중지
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.05f);
		}
	}

	// 캐릭터 캡슐 충돌 제거
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 스켈레탈 메시를 레그돌로 전환
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// 플레이어가 시체에 걸리지 않게 설정
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		MeshComp->SetAllBodiesSimulatePhysics(true);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
	}

	// 죽음 사운드
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DeathSound,
			GetActorLocation()
		);
	}

	// 게임 모드에 몬스터 사망 알림
	if (APFGameMode* GameMode =
		Cast<APFGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnMonsterKilled();
	}

	// 일정 시간 후 시체 제거
	if (BDestroyOnDeath)
	{
		SetLifeSpan(DestroyDelay);
	}
	
	if (GetWorld())
    {
    	APFGameMode* GameMode = Cast<APFGameMode>(GetWorld()->GetAuthGameMode());
    	if (GameMode)
    	{
    		GameMode->HandleMonsterDeath(GetActorLocation());
    	}
    }
}

void APFMonsterCharacter::ResetAttack()
{
	CanAttack = true;

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Ready"));
	
}

bool APFMonsterCharacter::IsMonsterDead() const
{
	return IsDead;
}

void APFMonsterCharacter::EnableAttackBox()
{
	if (!AttackBox)
	{
		return;
	}
	
	AttackBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackBox->UpdateOverlaps();
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] AttackBox ON"));
		
}

void APFMonsterCharacter::DisableAttackBox()
{
	if (!AttackBox ||
		AttackBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		return;
	}
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
	if (!OtherActor || OtherActor == this || IsDead || !bIsAttacking)
	{
		return;
	}
	
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (OtherActor != PlayerPawn)
	{
		return;
	}
	
	// 이번 공격에서 이미 맞은 액터면 중복 데미지 방지
	if (HitActorsThisAttack.Contains(OtherActor))
	{
		return;
	}

	// 데미지를 주기 전에 기록
	HitActorsThisAttack.Add(OtherActor);
	
	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Hit: %s"), *OtherActor->GetName());
	
	UGameplayStatics::ApplyDamage(
		OtherActor,
		AttackDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
	
	//사운드
	if (AttackHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			AttackHitSound,
			OtherActor->GetActorLocation()
		);
	}
}

void APFMonsterCharacter::OnDamaged(AActor* DamageCauser)
{
	
}

void APFMonsterCharacter::StartAttackHit()
{
	if (IsDead || !bIsAttacking)
	{
		return;
	}
	
	// 새로운 공격 타격 구간이므로 이전 기록 제거
	HitActorsThisAttack.Empty();

	EnableAttackBox();
}

void APFMonsterCharacter::EndAttackHit()
{
	DisableAttackBox();
}

void APFMonsterCharacter::FinishAttack()
{
	DisableAttackBox();
	
	HitActorsThisAttack.Empty();
	
	if (!IsDead)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
	
	bIsAttacking = false;
		
	UE_LOG(LogTemp, Warning, TEXT("[Monster] Attack Finished"));
}
