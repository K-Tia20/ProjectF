

#pragma once

#include "CoreMinimal.h"
#include "PFBasicZombie.h"
#include "PFSuicideZombie.generated.h"

UCLASS()
class PROJECTF_API APFSuicideZombie : public APFBasicZombie
{
	GENERATED_BODY()

public:
	
	APFSuicideZombie();

protected:
	
	virtual void BeginPlay() override;
	
	// 자폭 준비 시작
	void StartExplosion();

	// 실제 폭발 판정
	void Explode();

protected:
	
	// 폭발이 적용되는 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionRadius = 300.f;

	// 플레이어에게 주는 폭발 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionDamage = 60.f;

	// 보스 방어도에 주는 피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ArmorDamage = 50.f;

	// 자폭을 시작한 뒤 실제로 터질 때까지의 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuicideZombie|Explosion")
	float ExplosionDelay = 1.5f;

	// 이미 자폭을 시작했는지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuicideZombie|Explosion")
	bool bIsExploding = false;

	// 폭발 지연용 타이머
	FTimerHandle ExplosionTimerHandle;

public:
	
	
};
