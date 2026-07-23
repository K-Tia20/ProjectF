

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
#include "GameManager/PFPlayerController.h"
#include "Engine/EngineTypes.h"



//최신2026-07-22
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
	//왼손
	LeftAttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftAttackBox"));
	LeftAttackBox->SetupAttachment(GetMesh(), TEXT("AttackSocket_L"));

	LeftAttackBox->SetBoxExtent(FVector(30.f, 30.f, 30.f));
	LeftAttackBox->SetRelativeLocation(FVector::ZeroVector);
	LeftAttackBox->SetRelativeRotation(FRotator::ZeroRotator);

	LeftAttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftAttackBox->SetCollisionObjectType(ECC_WorldDynamic);
	LeftAttackBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftAttackBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LeftAttackBox->SetGenerateOverlapEvents(true);
	
}

void APFMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		InitialMeshRelativeTransform =
			MeshComp->GetRelativeTransform();

		InitialMeshCollisionProfileName =
			MeshComp->GetCollisionProfileName();

		InitialMeshCollisionEnabled =
			MeshComp->GetCollisionEnabled();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		InitialCapsuleCollisionProfileName =
			CapsuleComp->GetCollisionProfileName();

		InitialCapsuleCollisionEnabled =
			CapsuleComp->GetCollisionEnabled();
	}
	
	if (AttackBox && GetMesh())
	{
		AttackBox->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("AttackSocket_R")
		);
		
		LeftAttackBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&APFMonsterCharacter::OnAttackBoxOverlap
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
	PlayDamageSound();
	
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
	if (IsDead || !CanAttack || bIsAttacking || bIsKnockedDown)

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

void APFMonsterCharacter::ApplyExplosionKnockback(
	const FVector& ExplosionLocation,
	float ExplosionRadius,
	float KnockbackStrength
)
{
	// 죽었거나 보스이거나 이미 쓰러졌으면 중복 처리하지 않는다.
	if (IsDead || bIsBoss || bIsKnockedDown)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (!MeshComp)
	{
		return;
	}
	
	// 부분 피격 물리를 끝내고 전체 래그돌로 전환
	FinishBulletHitReaction();

	bIsKnockedDown = true;
	bIsAttacking = false;
	CanAttack = false;

	HitActorsThisAttack.Empty();

	// 공격 타이머와 공격 판정 정리
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackBoxTimerHandle);

	DisableAttackBox();
	DisableLeftAttackBox();

	// AI 이동 정지
	if (AController* MyController = GetController())
	{
		MyController->StopMovement();
	}

	// 캐릭터 이동 정지
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 재생 중인 몽타주 정지
	if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.05f);
	}

	// 캡슐은 물리 이동을 방해하지 않도록 잠시 끈다.
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
	}

	// 살아 있는 상태에서 임시 래그돌 전환
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetCollisionEnabled(
		ECollisionEnabled::QueryAndPhysics
	);

	MeshComp->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Ignore
	);

	MeshComp->SetAllBodiesSimulatePhysics(true);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();

	// 폭발 중심을 조금 아래로 내려서
	// 바깥쪽 + 위쪽 방향으로 날아가게 한다.
	const FVector ImpulseOrigin =
		ExplosionLocation - FVector(0.f, 0.f, 100.f);

	MeshComp->AddRadialImpulse(
		ImpulseOrigin,
		ExplosionRadius,
		KnockbackStrength,
		ERadialImpulseFalloff::RIF_Constant,
		true
	);
	
	// 일정 시간 후 날아간 위치에서 다시 일어난다.
	GetWorldTimerManager().ClearTimer(KnockdownTimerHandle);

	GetWorldTimerManager().SetTimer(
		KnockdownTimerHandle,
		this,
		&APFMonsterCharacter::RecoverFromKnockdown,
		KnockdownDuration,
		false
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Monster] Explosion Knockdown / Strength: %f"),
		KnockbackStrength
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
	GetWorldTimerManager().ClearTimer(KnockdownTimerHandle);
	GetWorldTimerManager().ClearTimer(GetUpTimerHandle);
	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);
	CurrentHitReactionBone = NAME_None;

	DisableAttackBox();
	DisableLeftAttackBox();
		
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
		GameMode->OnMonsterKilled(this);
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
	
	APFPlayerController* PlayerController = Cast<APFPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController)
	{
		PlayerController->ShowKillNotification();
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
	
	DisableLeftAttackBox();
	EnableAttackBox();
}

void APFMonsterCharacter::EndAttackHit()
{
	DisableAttackBox();
}

void APFMonsterCharacter::FinishAttack()
{
	DisableAttackBox();
	DisableLeftAttackBox();
	
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

void APFMonsterCharacter::EnableLeftAttackBox()
{
	if (!LeftAttackBox)
	{
		return;
	}

	LeftAttackBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LeftAttackBox->UpdateOverlaps();

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Left AttackBox ON"));
}
//왼손
void APFMonsterCharacter::DisableLeftAttackBox()
{
	if (!LeftAttackBox ||
		LeftAttackBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		return;
	}

	LeftAttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(LogTemp, Warning, TEXT("[Monster] Left AttackBox OFF"));
}

void APFMonsterCharacter::StartLeftAttackHit()
{
	if (IsDead || !bIsAttacking)
	{
		return;
	}

	// 오른손 타격 기록을 초기화
	HitActorsThisAttack.Empty();

	// 안전하게 오른손 박스는 끄고 왼손만 켠다.
	DisableAttackBox();
	EnableLeftAttackBox();
}

void APFMonsterCharacter::EndLeftAttackHit()
{
	DisableLeftAttackBox();
}

void APFMonsterCharacter::RecoverFromKnockdown()
{
	if (IsDead || !bIsKnockedDown)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();

	if (!MeshComp || !CapsuleComp)
	{
		return;
	}

	// 지정한 골반 뼈가 없으면 스켈레톤의 첫 번째 뼈 사용
	FName FollowBoneName = RagdollRootBoneName;

	if (MeshComp->GetBoneIndex(FollowBoneName) == INDEX_NONE)
	{
		FollowBoneName = MeshComp->GetBoneName(0);
	}

	// 실제로 날아간 래그돌의 현재 위치
	const FVector RagdollLocation =
		MeshComp->GetBoneLocation(FollowBoneName);
	
	// =====================================================
	// 엎드림 / 뒤로 누움 판정
	// =====================================================

	FName OrientationBoneName = GetUpOrientationBoneName;

	if (MeshComp->GetBoneIndex(OrientationBoneName) == INDEX_NONE)
	{
		OrientationBoneName = FollowBoneName;
	}

	const FTransform OrientationTransform =
		MeshComp->GetSocketTransform(
			OrientationBoneName,
			RTS_World
		);

	const FVector BodyFacingDirection =
		OrientationTransform.TransformVectorNoScale(
			GetUpFacingLocalAxis
		).GetSafeNormal();

	const float FaceUpDot =
		FVector::DotProduct(
			BodyFacingDirection,
			FVector::UpVector
		);

	// 몸의 앞쪽이 하늘을 향하면 등을 바닥에 대고 누운 상태
	const bool bIsLyingOnBack = FaceUpDot >= 0.f;

	// 뒤로 누웠을 때만 Back 몽타주 사용
	// 그 외에는 기존 Front 몽타주 사용
	UAnimMontage* SelectedGetUpMontage = GetUpMontage;

	if (bIsLyingOnBack && GetUpBackMontage)
	{
		SelectedGetUpMontage = GetUpBackMontage;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Monster] Get Up Type: %s / Dot: %f"),
		bIsLyingOnBack ? TEXT("Back") : TEXT("Front"),
		FaceUpDot
	);

	// 래그돌 아래의 지면 탐색
	FHitResult GroundHit;

	const FVector TraceStart =
		RagdollLocation + FVector(0.f, 0.f, 100.f);

	const FVector TraceEnd =
		RagdollLocation - FVector(0.f, 0.f, 500.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHitGround =
		GetWorld()->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

	// 캡슐 중심은 바닥보다 반높이만큼 위에 있어야 한다.
	FVector NewActorLocation = RagdollLocation;

	if (bHitGround)
	{
		NewActorLocation =
			GroundHit.ImpactPoint +
			FVector(
				0.f,
				0.f,
				CapsuleComp->GetScaledCapsuleHalfHeight()
			);
	}
	else
	{
		NewActorLocation.Z +=
			CapsuleComp->GetScaledCapsuleHalfHeight();
	}

	// 물리 속도 제거
	MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetPhysicsAngularVelocityInDegrees(
		FVector::ZeroVector
	);

	// 래그돌 물리 해제
	MeshComp->SetAllBodiesSimulatePhysics(false);
	MeshComp->SetSimulatePhysics(false);

	// 액터와 캡슐을 래그돌이 날아간 위치로 이동
	SetActorLocation(
		NewActorLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	// 메시를 캡슐에 다시 부착하고 원래 상대 위치로 복원
	MeshComp->AttachToComponent(
		CapsuleComp,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	MeshComp->SetRelativeTransform(
		InitialMeshRelativeTransform
	);

	// 원래 충돌 설정 복원
	MeshComp->SetCollisionProfileName(
		InitialMeshCollisionProfileName
	);

	MeshComp->SetCollisionEnabled(
		InitialMeshCollisionEnabled
	);

	CapsuleComp->SetCollisionProfileName(
		InitialCapsuleCollisionProfileName
	);

	CapsuleComp->SetCollisionEnabled(
		InitialCapsuleCollisionEnabled
	);
	
	// 총알 라인트레이스용 충돌을 확실하게 복구
	MeshComp->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block
	);

	// 래그돌 이후 물리 충돌 상태 새로 생성
	MeshComp->RecreatePhysicsState();
	MeshComp->UpdateBounds();

	// 머리 전용 히트박스도 다시 활성화
	const TArray<UActorComponent*> HeadHitBoxes =
		GetComponentsByTag(
			UBoxComponent::StaticClass(),
			TEXT("HeadHitBox")
		);

	for (UActorComponent* Component : HeadHitBoxes)
	{
		UBoxComponent* HeadHitBox =
			Cast<UBoxComponent>(Component);

		if (!HeadHitBox)
		{
			continue;
		}

		HeadHitBox->SetCollisionEnabled(
			ECollisionEnabled::QueryOnly
		);

		HeadHitBox->SetCollisionResponseToAllChannels(
			ECR_Ignore
		);

		HeadHitBox->SetCollisionResponseToChannel(
			ECC_Visibility,
			ECR_Block
		);

		HeadHitBox->UpdateOverlaps();
	}

	// 일어나는 동안에는 이동과 공격을 계속 막는다.
	bIsAttacking = true;
	CanAttack = false;

	// 일어나기 몽타주 재생
	float GetUpDuration = 0.f;

	if (SelectedGetUpMontage)
	{
		GetUpDuration =
			PlayAnimMontage(SelectedGetUpMontage);
	}

	// 몽타주가 정상적으로 재생됐다면
	// 재생 시간이 끝난 뒤 이동과 공격을 다시 활성화한다.
	if (GetUpDuration <= 0.f)
	{
		// 몽타주가 없거나 재생 실패했을 때만 바로 복구
		FinishGetUp();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Monster] Get Up Started / Duration: %f"),
		GetUpDuration
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[Monster] Recovered From Knockdown / Location: %s"
		),
		*NewActorLocation.ToString()
	);
}

void APFMonsterCharacter::FinishGetUp()
{
	if (IsDead)
	{
		return;
	}

	// 캐릭터 이동 다시 활성화
	if (UCharacterMovementComponent* Movement =
		GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	bIsKnockedDown = false;
	bIsAttacking = false;
	CanAttack = true;
	
	// 일반 좀비가 즉시 추적을 재개하도록 알림
	OnGetUpFinished();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Monster] Get Up Finished")
	);
}

void APFMonsterCharacter::OnGetUpFinished()
{
	
}

void APFMonsterCharacter::ApplyBulletHitReaction(
	FName HitBoneName,
	FVector HitDirection
)
{
	// 죽었거나 폭발 넉다운 중이면 부분 피격 반응을 실행하지 않음
	if (IsDead || bIsKnockedDown)
	{
		return;
	}

	// 맞은 본 이름이 없으면 실행할 수 없음
	if (HitBoneName.IsNone())
	{
		return;
	}
	
	// 골반부터 물리를 켜면 거의 전신이 흔들려
	// 메시가 캡슐 아래로 파고들 수 있으므로 제외
	if (HitBoneName == TEXT("Hips") ||
		HitBoneName == TEXT("Root"))
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (!MeshComp || !MeshComp->GetPhysicsAsset())
	{
		return;
	}

	// 이전 피격 반응 타이머 초기화
	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);

	// 이전 피격 반응이 남아 있다면
	// 같은 부위를 다시 맞아도 먼저 완전히 복구한다.
	if (!CurrentHitReactionBone.IsNone())
	{
		FinishBulletHitReaction();
	}

	CurrentHitReactionBone = HitBoneName;

	FVector ReactionDirection = HitDirection.GetSafeNormal();
		
	if (ReactionDirection.IsNearlyZero())
	{
		ReactionDirection = -GetActorForwardVector();
	}
	
	// 총알 피격 때문에 몸이 땅속으로 밀리지 않도록
	// 수직 방향의 충격을 제거
	ReactionDirection.Z = 0.f;
	ReactionDirection.Normalize();

	// 맞은 본과 그 아래 뼈들만 물리 활성화
	MeshComp->SetAllBodiesBelowSimulatePhysics(
		HitBoneName,
		true,
		true
	);

	// 애니메이션과 물리를 섞음
	MeshComp->SetAllBodiesBelowPhysicsBlendWeight(
		HitBoneName,
		HitReactionBlendWeight,
		false,
		true
	);

	// 맞은 방향으로 짧은 충격 적용
	MeshComp->AddImpulse(
		ReactionDirection * HitReactionImpulse,
		HitBoneName,
		true
	);

	// 잠시 후 다시 애니메이션으로 복귀
	GetWorldTimerManager().SetTimer(
		HitReactionTimerHandle,
		this,
		&APFMonsterCharacter::FinishBulletHitReaction,
		HitReactionDuration,
		false
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Monster] Bullet Hit Reaction / Bone: %s"),
		*HitBoneName.ToString()
	);
}

void APFMonsterCharacter::FinishBulletHitReaction()
{
	GetWorldTimerManager().ClearTimer(HitReactionTimerHandle);

	if (CurrentHitReactionBone.IsNone())
	{
		return;
	}

	const FName ReactionBone = CurrentHitReactionBone;
	CurrentHitReactionBone = NAME_None;

	USkeletalMeshComponent* MeshComp = GetMesh();

	if (!MeshComp)
	{
		return;
	}

	// 사망이나 전체 래그돌 상태에서는 물리를 끄면 안 됨
	if (IsDead || bIsKnockedDown)
	{
		return;
	}

	// 물리 혼합을 제거하고 애니메이션으로 복귀
	MeshComp->SetAllBodiesBelowPhysicsBlendWeight(
		ReactionBone,
		0.f,
		false,
		true
	);

	MeshComp->SetAllBodiesBelowSimulatePhysics(
		ReactionBone,
		false,
		true
	);
}


void APFMonsterCharacter::PlayDamageSound()
{
	if (!HitSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		HitSound,
		GetActorLocation()
	);
}