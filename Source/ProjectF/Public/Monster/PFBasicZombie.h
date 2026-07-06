
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

public:
	

	
};
