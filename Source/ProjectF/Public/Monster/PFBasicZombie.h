
#pragma once

#include "CoreMinimal.h"
#include "PFMonsterCharacter.h"
#include "PFBasicZombie.generated.h"

UCLASS()
class PROJECTF_API APFBasicZombie : public APFMonsterCharacter
{
	GENERATED_BODY()

public:
	APFBasicZombie();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Zombie")
	float DetectRange = 1000.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Zombie")
	float WalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float MoveAcceptanceRadius = 20.f;
	
	UPROPERTY(VisibleAnywhere)
	AActor* TargetActor;
	
	FTimerHandle DetectTimerHandle;
	
	void DetectPlayer();
	
	void ChasePlayer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
	float SightAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
	float RequiredDetectTime = 1.0f;

	float CurrentDetectTime = 0.f;

	bool IsPlayerInSight(AActor* Player);

	virtual void OnDamaged(AActor* DamageCauser) override;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDrawDebug = true;
	
	
public:
	

	
};
