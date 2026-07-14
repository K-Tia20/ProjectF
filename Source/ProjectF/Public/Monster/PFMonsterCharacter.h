

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
	
protected:
	virtual void BeginPlay() override;

	// 데미지를 받은 뒤 자식 몬스터가 추가 반응을 할 수 있게 만든 함수
	// 예: 좀비는 피격 시 공격자를 즉시 추적
	virtual void OnDamaged(AActor* DamageCauser);


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
			
};
