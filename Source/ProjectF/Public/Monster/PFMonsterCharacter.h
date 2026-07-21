

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PFMonsterCharacter.generated.h"

class UAnimMontage;
class USoundBase;

UCLASS()
class PROJECTF_API APFMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APFMonsterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Damage / Attack / Death =====
	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void TakeDamageFromMonster(float DamageAmount, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void Attack();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	bool IsMonsterDead() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	bool bIsBoss = false;//UI
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	bool bIsAttacking = false;//공격중 상태 확인 변수
	
	// 폭발에 맞았을 때 임시 래그돌과 넉백 적용
	UFUNCTION(BlueprintCallable, Category = "Monster|Knockdown")
	void ApplyExplosionKnockback(
		const FVector& ExplosionLocation,
		float ExplosionRadius,
		float KnockbackStrength
	);
	
	FTimerHandle GetUpTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Monster|Knockdown")
	void FinishGetUp();
	
	// 총알에 맞은 부위의 물리 피격 반응
	UFUNCTION(BlueprintCallable, Category = "Monster|HitReaction")
	void ApplyBulletHitReaction(
		FName HitBoneName,
		FVector HitDirection
	);
	
protected:
	virtual void BeginPlay() override;

	// 데미지를 받은 뒤 자식 몬스터가 추가 반응을 할 수 있게 만든 함수
	// 예: 좀비는 피격 시 공격자를 즉시 추적
	virtual void OnDamaged(AActor* DamageCauser);
	
	// 일어나기가 완료된 뒤 자식 몬스터가 추가 행동
	virtual void OnGetUpFinished();


	// =====================================================
	// Status
	// =====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	float CurrentHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackCooldown = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	bool IsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	bool CanAttack = true;
	
	// 현재 폭발에 쓰러진 상태인지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Knockdown")
	bool bIsKnockedDown = false;
	
	// 쓰러져 있는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Knockdown")
	float KnockdownDuration = 2.5f;

	// 래그돌 위치를 추적할 중심 뼈
	// Mixamo 계열이면 보통 Hips
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Knockdown")
	FName RagdollRootBoneName = TEXT("Hips");

	// 넉다운 복구 타이머
	FTimerHandle KnockdownTimerHandle;

	// 메시의 원래 상대 위치와 회전
	FTransform InitialMeshRelativeTransform;

	// 원래 충돌 설정
	FName InitialMeshCollisionProfileName;
	FName InitialCapsuleCollisionProfileName;

	ECollisionEnabled::Type InitialMeshCollisionEnabled =
		ECollisionEnabled::QueryOnly;

	ECollisionEnabled::Type InitialCapsuleCollisionEnabled =
		ECollisionEnabled::QueryAndPhysics;

	// 래그돌에서 복구
	void RecoverFromKnockdown();
	
public:

	// =====================================================
	// Attack
	// =====================================================

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	UBoxComponent* AttackBox = nullptr;//오른손 or 오리진
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	UBoxComponent* LeftAttackBox = nullptr;//왼손

	FTimerHandle AttackTimerHandle;
	FTimerHandle AttackBoxTimerHandle;

	void ResetAttack();
	//오른속 or 오리진
	void EnableAttackBox();
	void DisableAttackBox();
	//왼손
	void EnableLeftAttackBox();
	void DisableLeftAttackBox();

	UFUNCTION()
	void OnAttackBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	// 현재 공격에서 이미 데미지를 받은 액터들
	TSet<AActor*> HitActorsThisAttack;
	
public:	
	UFUNCTION(BlueprintCallable, Category = "Monster|Attack")
	void StartAttackHit();

	UFUNCTION(BlueprintCallable, Category = "Monster|Attack")
	void EndAttackHit();

	UFUNCTION(BlueprintCallable, Category = "Monster|Attack")
	void FinishAttack();
	//왼손
	UFUNCTION(BlueprintCallable, Category = "Monster|Attack")
	void StartLeftAttackHit();

	UFUNCTION(BlueprintCallable, Category = "Monster|Attack")
	void EndLeftAttackHit();

		
	// =====================================================
	// Death
	// =====================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Death")
	float DestroyDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Death")
	bool BDestroyOnDeath = true;
	
	// =====================================================
	// Evevt
	// =====================================================
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Event")
	void OnMonsterDamaged(float DamageAmount, AActor* DamageCauser);//UI호출용
	
	// =====================================================
	// Sound
	// =====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Sound")
	USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Sound")
	USoundBase* DeathSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Sound")
	USoundBase* AttackSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Sound")
	USoundBase* AttackHitSound = nullptr;
	
	// =====================================================
	// Animation
	// =====================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Animation")
	UAnimMontage* AttackMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Animation")
	UAnimMontage* GetUpMontage = nullptr;
	
	// =====================================================
	// Bullet Hit Reaction
	// =====================================================

	// 피격 부위에 가하는 충격 세기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|HitReaction")
	float HitReactionImpulse = 80.f;

	// 물리 반응이 유지되는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|HitReaction")
	float HitReactionDuration = 0.15f;

	// 애니메이션과 물리를 섞는 비율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|HitReaction",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitReactionBlendWeight = 0.35f;

	// 현재 물리 반응 중인 본
	FName CurrentHitReactionBone = NAME_None;

	// 피격 반응 종료 타이머
	FTimerHandle HitReactionTimerHandle;

	// 피격 반응을 종료하고 애니메이션으로 복귀
	void FinishBulletHitReaction();
			
};
