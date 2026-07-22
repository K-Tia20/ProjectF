#include "Character/PFPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"   
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PFCombatComponent.h"    
#include "GameManager/PFPlayerController.h"
#include "GameManager/PFGameInstance.h" 
#include "GameManager/PFLevelPortal.h"      
#include "GameManager/PFGameMode.h"         
#include "Animation/AnimInstance.h"
#include "Character/PFDashComponent.h"
#include "Kismet/GameplayStatics.h" 

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

    CombatComponent = CreateDefaultSubobject<UPFCombatComponent>(TEXT("CombatComponent"));
    DashComponent = CreateDefaultSubobject<UPFDashComponent>(TEXT("DashComponent"));
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

    
    if (CombatComponent)
    {
        CombatComponent->SwitchWeapon(EWeaponType::Pistol);
    }
}

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

    if (APFPlayerController* PlayerController = Cast<APFPlayerController>(GetController()))
    {
        PlayerController->UpdatePlayerHP(Health, MaxHealth); 

        if (ActualDamage > 0.0f)
        {
            PlayerController->ShowBloodEffect(ActualDamage);
        }
    }
    if (Health <= 0.0f)
    {
        Health = 0.0f;
        Die(); 
    }

    return ActualDamage; 
}

void APFPlayerCharacter::Die()
{
    bIsDead = true;
    UE_LOG(LogTemp, Warning, TEXT("플레이어 사망!"));

    GetCharacterMovement()->DisableMovement();

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    }

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

// [수정] 기존 Fire 함수 대신 StartFire 구현 (마우스 버튼을 눌렀을 때)
void APFPlayerCharacter::StartFire(const FInputActionValue& Value)
{
    if (bIsDead) return;

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
        CombatComponent->StartFire(); 
    }
}


void APFPlayerCharacter::StopFire(const FInputActionValue& Value)
{
    if (bIsDead) return;

    if (CombatComponent)
    {
        CombatComponent->StopFire(); 
    }
}

void APFPlayerCharacter::OnDashInput(const FInputActionValue& Value)
{
    if (bIsDead) return;

    if (DashComponent)
    {
        DashComponent->ExecuteDash();
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
        
        // [수정] FireAction을 Started(누름) 및 Completed(뗌)로 각각 바인딩
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APFPlayerCharacter::StartFire);  // [수정]
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APFPlayerCharacter::StopFire); // [추가]

        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APFPlayerCharacter::OnInteract);
        EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &APFPlayerCharacter::ThrowGrenade);
        EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &APFPlayerCharacter::ChangeWeaponInput);
        EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APFPlayerCharacter::OnDashInput);
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


void APFPlayerCharacter::ChangeWeaponInput(const FInputActionValue& Value)
{
    if (bIsDead || !CombatComponent) return;

    float WeaponIndex = Value.Get<float>();
    bool bSuccess = false;
    
    if (WeaponIndex == 1.0f)
    {
        bSuccess = CombatComponent->SwitchWeapon(EWeaponType::Pistol);
    }
    else if (WeaponIndex == 2.0f)
    {
        bSuccess = CombatComponent->SwitchWeapon(EWeaponType::Rifle);
    }
    else if (WeaponIndex == 3.0f)
    {
        bSuccess = CombatComponent->SwitchWeapon(EWeaponType::Sword);
    }

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] 현재 레벨에서는 해당 무기를 사용할 수 없습니다!"));
    }
}


void APFPlayerCharacter::PlayFootstepSound()
{
   
    if (bIsDead || GetCharacterMovement()->IsFalling())
    {
        return;
    }

   
    if (FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
    }
}