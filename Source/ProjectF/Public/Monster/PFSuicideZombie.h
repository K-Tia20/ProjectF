

#pragma once

#include "CoreMinimal.h"
#include "PFBasicZombie.h"
#include "PFSuicideZombie.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class PROJECTF_API APFSuicideZombie : public APFBasicZombie
{
	GENERATED_BODY()

public:
	
	APFSuicideZombie();
	
	virtual void Attack() override;

protected:
	
	virtual void BeginPlay() override;
	
	// 자폭 준비 시작
	void StartExplosion();

	// 실제 폭발 판정
	void Explode();

	// 폭발이 적용되는 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionRadius = 500.f;

	// 플레이어에게 주는 폭발 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionDamage = 60.f;

	// 보스 방어도에 주는 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ArmorDamage = 50.f;

	// 자폭을 시작한 뒤 실제로 터질 때까지의 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionDelay = 1.5f;
	
	// 폭발 순간 생성할 Niagara 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Explosion")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	// 이미 자폭을 시작했는지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Explosion")
	bool bIsExploding = false;

	// 폭발 지연용 타이머
	FTimerHandle ExplosionTimerHandle;
	
	// 폭발 범위 디버그 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Debug")
	bool bDrawExplosionRange = true;
	
	virtual void Die() override;

	// 이미 실제 폭발을 완료했는지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Explosion")
	bool bHasExploded = false;

	// 죽은 뒤 폭탄 상태로 남아 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Explosion")
	bool bIsCorpseBomb = false;
	
	// 복부 약점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|WeakPoint")
	UBoxComponent* WeakPointCollision = nullptr;

	// 가슴 약점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|WeakPoint")
	UBoxComponent* WeakPointCollision2 = nullptr;
	
	// 자폭 준비 중 재생할 비웃음·심장 고동 소리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Sound")
	TObjectPtr<USoundBase> ChargeSound;

	// 실제 폭발 순간 재생할 소리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Sound")
	TObjectPtr<USoundBase> ExplosionSound;
	

public:
	
	// 약점 코어를 맞았을 때 호출
	UFUNCTION(BlueprintCallable, Category = "SuicideZombie|Explosion")
	void TriggerWeakPointExplosion();
	
	UFUNCTION(BlueprintPure, Category = "SuicideZombie|WeakPoint")
	bool IsWeakPointComponent(const UPrimitiveComponent* HitComponent) const;

	// 자폭 경고가 시작됐을 때 BP 연출 실행
	UFUNCTION(BlueprintImplementableEvent, Category = "SuicideZombie|Visual")
	void OnExplosionWarningStarted();
	
	// 실제 폭발 순간 BP에 알림
	UFUNCTION(BlueprintImplementableEvent, Category = "SuicideZombie|Visual")
	void OnSuicideExploded();
	
	UFUNCTION(BlueprintPure, Category = "SuicideZombie|WeakPoint")
	bool IsWeakPointLocation(const FVector& WorldLocation) const;
	
};
