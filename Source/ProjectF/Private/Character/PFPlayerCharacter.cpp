#include "Character/PFPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PFCombatComponent.h"
#include "GameManager/PFPlayerController.h"
#include "GameManager/PFGameInstance.h" 
#include "GameManager/PFLevelPortal.h"      
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

    // (컴뱃 컴포넌트 관련) 캐릭터에 컴뱃 컴포넌트 부착
    CombatComponent = CreateDefaultSubobject<UPFCombatComponent>(TEXT("CombatComponent"));
}

float APFPlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health -= ActualDamage;

   
    if (Health <= 0.0f)
    {
        Health = 0.0f;
    }

    UE_LOG(LogTemp, Warning, TEXT("현재 체력: %f"), Health);

   
    if (APFPlayerController* PlayerController = Cast<APFPlayerController>(Controller))
    {
        PlayerController->UpdatePlayerHP(Health, MaxHealth); 
    }

    return ActualDamage;
}


void APFPlayerCharacter::Heal(float HealAmount)
{
    Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("[Player] 힐팩 먹음! 현재 체력: %f / %f"), Health, MaxHealth);
    
    
    if (APFPlayerController* PlayerController = Cast<APFPlayerController>(GetController()))
    {
        PlayerController->UpdatePlayerHP(Health, MaxHealth);
    }
}

void APFPlayerCharacter::Fire(const FInputActionValue& Value)
{
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

    // 기존 IMC 등록 코드
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

       
EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APFPlayerCharacter::OnInteract);    }
}

// [추가] 상호작용 함수 구현
void APFPlayerCharacter::OnInteract()
{
    // 포탈 근처 영역에 서 있다면
    if (TargetPortal)
    {
        // 포탈을 가동
        TargetPortal->Interact(this);
    }
}

void APFPlayerCharacter::Move(const FInputActionValue& Value)
{
    // 공중에 있거나 착지 애니메이션 중에는 이동 입력을 차단하여 문워크 방지
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

