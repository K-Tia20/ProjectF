#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PFCombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTF_API UPFCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPFCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 블루프린트나 캐릭터 클래스에서 호출할 발사 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 25.f;
};