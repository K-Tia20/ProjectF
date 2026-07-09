#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PFPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPFCombatComponent;

UCLASS()
class PROJECTF_API APFPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APFPlayerCharacter();

	// [추가] 현재 체력 상태를 저장할 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health = 100.0f;

	// [추가] 최대 체력 설정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.0f;
	
	// [추가] 피격 시 엔진에서 자동으로 호출되는 기본 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public: 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPFCombatComponent* CombatComponent;
    
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	
	void Fire(const FInputActionValue& Value);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
};