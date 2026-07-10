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
class APFLevelPortal; // 포탈 클래스 전방 선언

UCLASS()
class PROJECTF_API APFPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APFPlayerCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    //  캐릭터가 죽었는지 체크하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsDead = false;

    // 캐릭터 사망 시 호출될 함수
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    // 힐 아이템 사용시 호출
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Heal(float HealAmount);

    
    void SetTargetPortal(class APFLevelPortal* Portal) { TargetPortal = Portal; }
    class APFLevelPortal* GetTargetPortal() const { return TargetPortal; }

protected:
    virtual void BeginPlay() override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // 상호작용 인풋 액션 변수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    // F키(상호작용)를 눌렀을 때 실행될 함수
    void OnInteract();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* FireMontage;

    // 현재 플레이어가 닿아있는 포탈을 보관할 변수
    UPROPERTY()
    APFLevelPortal* TargetPortal;
};