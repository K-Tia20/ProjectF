#include "Character/PFCombatComponent.h"
//#include "DrawDebugHelpers.h"
#include "GameManager/PFPlayerController.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "weapon/PFGrenade.h"
#include "TimerManager.h" 
#include "Engine/OverlapResult.h"
#include "Monster/PFSuicideZombie.h"
#include "Kismet/GameplayStatics.h" 
#include "Components/BoxComponent.h"
//#include "GameManager/PFGameState.h"  

UPFCombatComponent::UPFCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    HandGrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandGrenadeMesh"));
}

void UPFCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    SwitchWeapon(EWeaponType::Pistol);

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetMesh() && HandGrenadeMesh)
    {
        HandGrenadeMesh->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            HandSocketName
        );
        HandGrenadeMesh->SetVisibility(false);
        HandGrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UPFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPFCombatComponent::EquipWeapon(UStaticMesh* WeaponMesh)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (CurrentWeaponMesh)
    {
        CurrentWeaponMesh->DestroyComponent();
        CurrentWeaponMesh = nullptr; 
    }

    if (!WeaponMesh)
    {
        return; 
    }
    
    CurrentWeaponMesh = NewObject<UStaticMeshComponent>(Owner);
    CurrentWeaponMesh->RegisterComponent();
    CurrentWeaponMesh->SetStaticMesh(WeaponMesh);

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        FName TargetSocketName = TEXT("WeaponSocket"); 

        if (CurrentWeapon == EWeaponType::Sword)
        {
            TargetSocketName = TEXT("SwordSocket"); 
        }

        CurrentWeaponMesh->AttachToComponent(
            Character->GetMesh(), 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
            TargetSocketName
        );
    }
}

bool UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
    StopFire();

    switch (CurrentWeapon)
    {
    case EWeaponType::Pistol:
        PistolCurrentAmmo = CurrentAmmo;
        break;
    case EWeaponType::Rifle:
        RifleCurrentAmmo = CurrentAmmo;
        break;
    case EWeaponType::Sword:
        SwordCurrentAmmo = CurrentAmmo;
        break;
    }
    
    CurrentWeapon = NewWeapon;
    
    AActor* Owner = GetOwner();
    if (Owner == nullptr) return false;
    
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        if (CurrentWeapon == EWeaponType::Pistol)
        {
            if (CurrentWeaponMesh)
            {
                CurrentWeaponMesh->DestroyComponent();
                CurrentWeaponMesh = nullptr;
            }
            Character->GetMesh()->UnHideBoneByName(DefaultPistolBoneName);
        }
        else 
        {
            Character->GetMesh()->HideBoneByName(DefaultPistolBoneName, EPhysBodyOp::PBO_None);
            
            if (CurrentWeaponMesh)
            {
                FName TargetSocketName = TEXT("WeaponSocket");
                
                if (CurrentWeapon == EWeaponType::Sword)
                {
                    TargetSocketName = TEXT("SwordSocket");
                }
                
                CurrentWeaponMesh->AttachToComponent(
                    Character->GetMesh(), 
                    FAttachmentTransformRules::SnapToTargetIncludingScale, 
                    TargetSocketName
                );
            }
        }
    }
   
    APawn* OwnerPawn = Cast<APawn>(Owner);
    APFPlayerController* PlayerController = OwnerPawn ? Cast<APFPlayerController>(OwnerPawn->GetController()) : nullptr;
    
    if (PlayerController != nullptr)
    {
        FString NameToShow = TEXT("기본 무기");
        if (CurrentWeapon == EWeaponType::Rifle)
        {
            NameToShow = TEXT("Assault Rifle");
        }
        else if (CurrentWeapon == EWeaponType::Pistol)
        {
            NameToShow = TEXT("Pistol");
        }
        else if (CurrentWeapon == EWeaponType::Sword)
        {
            NameToShow = TEXT("Sword");
        }

        PlayerController->UpdateWeaponNameUI(NameToShow);
    }
    
    switch (CurrentWeapon)
    {
    case EWeaponType::Pistol:
        MaxAmmo = 15;
        CurrentAmmo = PistolCurrentAmmo;
        AttackDamage = 15.f;
        AttackRange = 3000.f;
        FireRate = 0.25f;
        bIsAutomatic = false; 
        break;
    case EWeaponType::Rifle:
        MaxAmmo = 30;
        CurrentAmmo = RifleCurrentAmmo;
        AttackDamage = 25.f;
        AttackRange = 5000.f;
        FireRate = 0.1f;       
        bIsAutomatic = true;  
        break;
    case EWeaponType::Sword:
        MaxAmmo = 5;
        CurrentAmmo = SwordCurrentAmmo;
        AttackDamage = 100.f;
        AttackRange = 150.f; 
        FireRate = 1.0f;
        bIsAutomatic = false; 
    }
    
    if (PlayerController != nullptr)
    {
        PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo); 
    }

    return true; 
}


void UPFCombatComponent::StartFire()
{
    FireWeapon();

    if (bIsAutomatic && CurrentAmmo > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(
            AutoFireTimerHandle,
            this,
            &UPFCombatComponent::FireWeapon,
            FireRate, 
            true      
        );
    }
}


void UPFCombatComponent::StopFire()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

void UPFCombatComponent::AddAmmo(EWeaponType WeaponType, int32 Amount)
{
    if (CurrentWeapon == WeaponType)
    {
        CurrentAmmo = FMath::Clamp(CurrentAmmo + Amount, 0, MaxAmmo);
        AActor* Owner = GetOwner();
        if (Owner)
        {
            if (APawn* OwnerPawn = Cast<APawn>(Owner))
            {
                if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
                {
                    PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo);
                }
            }
        }
    }
}

void UPFCombatComponent::PlayThrowAnimation()
{
    if (GrenadeCount <= 0) return;

    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);
    
    if (Character && ThrowMontage)
    {
        if (HandGrenadeMesh)
        {
            HandGrenadeMesh->SetVisibility(true);
        }
        Character->PlayAnimMontage(ThrowMontage);
    }
}

void UPFCombatComponent::ExecuteThrow()
{
    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);
    
    if (!Character || !GrenadeClass || GrenadeCount <= 0 || bIsGrenadeOnCooldown) return;

    if (HandGrenadeMesh)
    {
        HandGrenadeMesh->SetVisibility(false);
    }

    GrenadeCount--;

    FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(HandSocketName); 
    FRotator SpawnRotation = Character->GetControlRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Character;

    GetWorld()->SpawnActor<APFGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, SpawnParams);

    bIsGrenadeOnCooldown = true;

    APFPlayerController* PlayerController = Cast<APFPlayerController>(Character->GetController());
    if (PlayerController)
    {
        PlayerController->UpdateGrenadeCooldownUI(GrenadeCooldown); 
    }

    GetWorld()->GetTimerManager().SetTimer(
        GrenadeCooldownTimerHandle, 
        this, 
        &UPFCombatComponent::ResetGrenadeCooldown, 
        GrenadeCooldown, 
        false
    );
}


void UPFCombatComponent::FireWeapon()
{
    if (CurrentAmmo <= 0)
    {
        StopFire(); 
        return;
    }

    if (!bIsAutomatic && !bCanFire) return; 
    
    CurrentAmmo--;

    if (!bIsAutomatic) 
    {
        bCanFire = false;
        GetWorld()->GetTimerManager().SetTimer(
            FireTimerHandle,
            this,
            &UPFCombatComponent::ResetFire,
            FireRate, 
            false
        );
    }

    AActor* Owner = GetOwner();

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            
            if (CurrentWeapon == EWeaponType::Pistol)
            {
                if (PistolFireMontage)
                {
                    AnimInstance->Montage_Play(PistolFireMontage, 1.0f);
                }

                if (PistolFireSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, PistolFireSound, Character->GetActorLocation());
                }
            }
            
            else if (CurrentWeapon == EWeaponType::Rifle)
            {
                if (RifleFireMontage)
                {
                    AnimInstance->Montage_Play(RifleFireMontage, 1.0f);
                }

                if (RifleFireSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(this, RifleFireSound, Character->GetActorLocation());
                }
            }
            else if (CurrentWeapon == EWeaponType::Sword && SwordMontage)
            {
                AnimInstance->Montage_Play(SwordMontage, 1.0f);
            }
        }
    }

    if (Owner)
    {
        if (APawn* OwnerPawn = Cast<APawn>(Owner))
        {
            if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
            {
                PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo);
            }
        }
    }

    if (CurrentWeapon == EWeaponType::Sword)
    {
        return; 
    }

    if (!Owner) return;

    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (!OwnerPawn || !OwnerPawn->GetController()) return;

    FVector StartLocation;
    FRotator StartRotation;
    OwnerPawn->GetController()->GetPlayerViewPoint(StartLocation, StartRotation);

    FVector EndLocation = StartLocation + (StartRotation.Vector() * AttackRange);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Owner); 

    bool bHit = GetWorld()->LineTraceSingleByChannel(
       HitResult,
       StartLocation,
       EndLocation,
       ECC_Visibility,
       CollisionParams
    );
    
    // 총알 방향과 명중 결과가 결정된 뒤 카메라 흔들림
    if (APFPlayerController* PlayerController =
        Cast<APFPlayerController>(OwnerPawn->GetController()))
    {
        if (FireCameraShakeClass &&
            PlayerController->IsLocalController())
        {
            PlayerController->ClientStartCameraShake(FireCameraShakeClass);
        }
    }
    
    if (Character && CurrentWeapon != EWeaponType::Sword)
    {
        float RecoilAmount = (CurrentWeapon == EWeaponType::Rifle) ? -0.8f : -1.2f;
        Character->AddControllerPitchInput(RecoilAmount);
    }
        
    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
            if (Monster)
            {
                FName HitBoneName = HitResult.BoneName;
                UPrimitiveComponent* HitComponent = HitResult.GetComponent();
                bool bWeakPointHit = false;
                
                bool bHitHeadBoxLocation = false;

                const TArray<UActorComponent*> HeadBoxComponents =
                    Monster->GetComponentsByTag(
                        UBoxComponent::StaticClass(),
                        TEXT("HeadHitBox")
                    );

                for (UActorComponent* Component : HeadBoxComponents)
                {
                    const UBoxComponent* HeadBox =
                        Cast<UBoxComponent>(Component);

                    if (!HeadBox)
                    {
                        continue;
                    }

                    const FVector LocalHitLocation =
                        HeadBox->GetComponentTransform()
                            .InverseTransformPosition(
                                HitResult.ImpactPoint
                            );

                    const FVector Extent =
                        HeadBox->GetUnscaledBoxExtent() +
                        FVector(2.f);

                    bHitHeadBoxLocation =
                        FMath::Abs(LocalHitLocation.X) <= Extent.X &&
                        FMath::Abs(LocalHitLocation.Y) <= Extent.Y &&
                        FMath::Abs(LocalHitLocation.Z) <= Extent.Z;

                    if (bHitHeadBoxLocation)
                    {
                        break;
                    }
                }
                
                // 머리 본 또는 머리 전용 박스를 맞혔는지 확인
                const bool bHitHeadBox =
                    HitComponent &&
                    HitComponent->ComponentHasTag(TEXT("HeadHitBox"));

                const bool bHeadShot =
                    HitBoneName == TEXT("Head") ||
                        bHitHeadBox ||
                        bHitHeadBoxLocation;
                
                // 머리 박스를 맞힌 경우 피격 리액션용 본을 Head로 지정
                const FName ReactionBoneName =
                     bHeadShot ? FName(TEXT("Head")) : HitBoneName;
                
                UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                "[HeadCheck] Boxes: %d / DirectBox: %d / LocationBox: %d / HeadShot: %d / Component: %s / Bone: %s"
                ),
                HeadBoxComponents.Num(),
                bHitHeadBox,
                bHitHeadBoxLocation,
                bHeadShot,
                *GetNameSafe(HitComponent),
                *HitBoneName.ToString()
                );

                if (APFSuicideZombie* SuicideZombie = Cast<APFSuicideZombie>(Monster))
                {
                    const bool bHitWeakPointComponent =
                    SuicideZombie->IsWeakPointComponent(HitComponent);

                    const bool bHitWeakPointLocation =
                        SuicideZombie->IsWeakPointLocation(
                            HitResult.ImpactPoint
                        );

                    if (bHitWeakPointComponent || bHitWeakPointLocation)
                    {
                        UE_LOG(
                            LogTemp,
                            Warning,
                            TEXT(
                                "자폭 좀비 약점 명중 / Component: %s / LocationCheck: %d"
                            ),
                            *GetNameSafe(HitComponent),
                            bHitWeakPointLocation
                        );

                        SuicideZombie->TriggerWeakPointExplosion();
                        bWeakPointHit = true;
                    }
                }

                if (!bWeakPointHit) 
                {
                    // 총알이 날아간 방향
                    const FVector HitDirection =
                        (HitResult.ImpactPoint - StartLocation).GetSafeNormal();
                    
                    if (bHeadShot)
                    {
                        UE_LOG(
                            LogTemp,
                            Warning,
                            TEXT("[Combat] Head Shot / Component: %s / Bone: %s"),
                            *GetNameSafe(HitComponent),
                            *HitBoneName.ToString()
                        );

                        Monster->TakeDamageFromMonster(
                            AttackDamage * 2.f,
                            Owner
                        );
                    }
                    else
                    {
                        Monster->TakeDamageFromMonster(
                            AttackDamage,
                            Owner
                        );
                    }
                    
                    // 총알 피해로 죽지 않은 경우에만 맞은 부위 흔들기
                    if (!Monster->IsMonsterDead())
                    {
                        Monster->ApplyBulletHitReaction(
                            ReactionBoneName,
                            HitDirection
                        );
                    }

                    UE_LOG(
                        LogTemp,
                        Warning,
                        TEXT("[Combat] Hit Reaction / Component: %s / Bone: %s"),
                        *GetNameSafe(HitComponent),
                        *HitBoneName.ToString()
                    );
                
                }
                
                if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
                {
                    PlayerController->ShowHitMarker();
                }
            }
        }
    }
}


void UPFCombatComponent::ResetFire()
{
    bCanFire = true;
}

void UPFCombatComponent::ExecuteMeleeHitCheck()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector StartLocation = Owner->GetActorLocation() + (Owner->GetActorForwardVector() * 100.f); 
    float AttackRadius = 150.f; 

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Owner); 

    bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        StartLocation,
        FQuat::Identity,
        ECC_Pawn, 
        FCollisionShape::MakeSphere(AttackRadius),
        CollisionParams
    );

    if (bHit)
    {
        TArray<AActor*> HitActors;

        for (const FOverlapResult& Overlap : OverlapResults)
        {
            AActor* HitActor = Overlap.GetActor();
            
            if (HitActor && !HitActors.Contains(HitActor))
            {
                APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
                if (Monster)
                {
                    Monster->TakeDamageFromMonster(AttackDamage, Owner);
                    HitActors.Add(HitActor);
                    
                    UE_LOG(LogTemp, Warning, TEXT("광역 베기 적중: %s"), *HitActor->GetName());
                }
            }
        }
    }
}

void UPFCombatComponent::AddGrenades(int32 Amount)
{
    if (Amount <= 0) return;
    GrenadeCount += Amount;
    GrenadeCount = FMath::Clamp(GrenadeCount, 0, 3);
}