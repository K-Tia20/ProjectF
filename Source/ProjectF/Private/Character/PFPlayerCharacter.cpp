#include "Character/PFPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"    // [추가된 코드] UCapsuleComponent 제어를 위해 인클루드 추가
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PFCombatComponent.h"    // EWeaponType 사용을 위해 이미 포함되어 있음
#include "GameManager/PFPlayerController.h"
#include "GameManager/PFGameInstance.h" 
#include "GameManager/PFLevelPortal.h"      
#include "GameManager/PFGameMode.h"         
#include "Animation/AnimInstance.h"


APFPlayerCharacter::APFPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true; 
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false; 
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
    GetCharacterMovement()->JumpZVelocity = 450.f;
    GetCharacterMovement()->AirControl = 0.2f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; 
    CameraBoom->bUsePawnControlRotation = true; 
    CameraBoom->SocketOffset = FVector(0.0f, 60.0f, 60.0f); 

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; 

    // 캐릭터에 컴뱃 컴포넌트 부착
    CombatComponent = CreateDefaultSubobject<UPFCombatComponent>(TEXT("CombatComponent"));
}

// 히트마커 호출 구현시 주석 제거 
//void APFPlayerCharacter::ShowHitMarker()
//{
   // if (APFPlayerController* PC = Cast<APFPlayerController>(GetController()))
   // {
      //  PC->ShowHitMarker();
    //}
// }

// 수류탄 던지기 명령 실행 함수
void APFPlayerCharacter::ThrowGrenade()
{
    if (bIsDead) return;
    
    if (CombatComponent)
    {
        CombatComponent->PlayThrowAnimation();
    }
}

float APFPlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health -= ActualDamage;

    if (Health <= 0.0f)
    {
        Health = 0.0f;
        Die(); 
    }

    UE_LOG(LogTemp, Warning, TEXT("현재 체력: %f"), Health);

    if (APFPlayerController* PlayerController = Cast<APFPlayerController>(Controller))
    {
        PlayerController->UpdatePlayerHP(Health, MaxHealth); 
    }

    return ActualDamage;
}

void APFPlayerCharacter::Die()
{
    bIsDead = true;
    UE_LOG(LogTemp, Warning, TEXT("플레이어 사망!"));

    GetCharacterMovement()->DisableMovement();

    // [추가된 코드] 사망 시 캡슐 컴포넌트 충돌 제거
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    // [추가된 코드] 랙돌 물리를 켜서 전신을 바닥에 눕힙니다.
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    }

    // 사망 시 현재 월드의 게임모드를 가져와 GameOver() 함수를 호출합니다.
    if (GetWorld())
    {
        if (APFGameMode* GameMode = Cast<APFGameMode>(GetWorld()->GetAuthGameMode()))
        {
            GameMode->GameOver();
        }
    }
}

void APFPlayerCharacter::Heal(float HealAmount)
{
    if (bIsDead) return;

    Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("[Player] 힐팩 먹음! 현재 체력: %f / %f"), Health, MaxHealth);
    
    if (APFPlayerController* PlayerController = Cast<APFPlayerController>(GetController()))
    {
        PlayerController->UpdatePlayerHP(Health, MaxHealth);
    }
}

void APFPlayerCharacter::Fire(const FInputActionValue& Value)
{
    if (bIsDead) return;

    UE_LOG(LogTemp, Warning, TEXT("Fire 함수가 호출되었습니다!"));
    
    if (FireMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(FireMontage, 1.0f); 
        }
    }

    if (CombatComponent)
    {
        CombatComponent->FireWeapon();
    }
}

void APFPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    if (APFPlayerController* PFController = Cast<APFPlayerController>(Controller))
    {
        PFController->UpdatePlayerHP(Health, MaxHealth);
    }

    if (UPFGameInstance* GameInstance = Cast<UPFGameInstance>(GetGameInstance()))
    {
        GameInstance->LoadPlayerState(this);
    }
}

void APFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APFPlayerCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APFPlayerCharacter::Look);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APFPlayerCharacter::Fire);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APFPlayerCharacter::OnInteract);
        
        // [추가된 코드] G키(또는 할당된 키) 입력 시 ThrowGrenade 함수가 호출되도록 바인딩
        EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &APFPlayerCharacter::ThrowGrenade);

        // [추가된 코드] 무기 교체 인풋 바인딩
        EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &APFPlayerCharacter::ChangeWeaponInput);
    }
}

void APFPlayerCharacter::OnInteract()
{
    if (bIsDead) return;

    if (TargetPortal)
    {
        TargetPortal->Interact(this);
    }
}

void APFPlayerCharacter::Move(const FInputActionValue& Value)
{
    if (GetCharacterMovement()->IsFalling()) 
    {
        return; 
    }

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
       const FRotator Rotation = Controller->GetControlRotation();
       const FRotator YawRotation(0, Rotation.Yaw, 0);

       const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
       const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

       AddMovementInput(ForwardDirection, MovementVector.Y);
       AddMovementInput(RightDirection, MovementVector.X);
    }
}

void APFPlayerCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

// 캐릭터에서 무기 교체 키 입력이 들어왔을 때 실행될 로직
void APFPlayerCharacter::ChangeWeaponInput(const FInputActionValue& Value)
{
    if (bIsDead) return;

    if (CombatComponent)
    {
       
        float WeaponIndex = Value.Get<float>();
        
        if (WeaponIndex == 1.0f)
        {
            CombatComponent->SwitchWeapon(EWeaponType::Pistol);
        }
        else if (WeaponIndex == 2.0f)
        {
            CombatComponent->SwitchWeapon(EWeaponType::Rifle);
        }
        else if (WeaponIndex == 3.0f)
        {
            CombatComponent->SwitchWeapon(EWeaponType::Sword);
        }
    }
}