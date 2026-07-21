#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PFDashComponent.generated.h"


class UNiagaraSystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTF_API UPFDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPFDashComponent();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ExecuteDash();

protected:
	virtual void BeginPlay() override;

	void ResetDash();
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashImpulse = 2500.f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashCooldown = 1.0f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* DashNiagaraVFX;

private:
	bool bCanDash = true;
	FTimerHandle DashTimerHandle;
};