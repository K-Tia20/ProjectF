#include "Character/PFCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"     // [추가] Cast<ACharacter> 및 구조 활용을 위해 필수 인클루드
#include "Components/StaticMeshComponent.h" // [추가] 가시성 및 컴포넌트 제어를 위해 필수 인클루드
#include "weapon/PFGrenade.h"

// ---------------------------------------------------------
// [추가된 코드] 팀원 요청: 레벨 정보를 가져오기 위해 GameState 헤더 추가
// (프로젝트 폴더 구조에 맞게 경로 확인 필요. 예: GameManager/)
// ---------------------------------------------------------
#include "GameManager/PFGameState.h" 

UPFCombatComponent::UPFCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // [추가] 생성자 시점에서 눈속임용 수류탄 메쉬의 기본 뼈대 컴포넌트 생성
    HandGrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandGrenadeMesh"));
}

void UPFCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // [추가] 게임 시작 시 주인 캐릭터의 메쉬 소켓에 눈속임용 메쉬를 안전하게 부착
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetMesh() && HandGrenadeMesh)
    {
        HandGrenadeMesh->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            HandSocketName
        );
        
        // 평소에는 보이지 않도록 숨기고, 충돌 판정을 꺼서 버그를 방지합니다.
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

    // 기존에 무기가 있다면 제거
    if (CurrentWeaponMesh)
    {
        CurrentWeaponMesh->DestroyComponent();
    }

    // 새로운 무기 메시 컴포넌트 생성
    CurrentWeaponMesh = NewObject<UStaticMeshComponent>(Owner);
    CurrentWeaponMesh->RegisterComponent();
    CurrentWeaponMesh->SetStaticMesh(WeaponMesh);

    // 캐릭터 손 소켓에 붙이기 
    USkeletalMeshComponent* CharacterMesh = Cast<ACharacter>(Owner)->GetMesh();
    CurrentWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
}

// 무기 교체 로직 무기마다 사거리 데미지가 다름 
void UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
    
    // 총기락
   
    int32 CurrentLevel = 1; 
    // 기본값
    if (GetWorld())
    {
        if (APFGameState* GameState = GetWorld()->GetGameState<APFGameState>())
        {   
            CurrentLevel = GameState->GetCurrentLevel(); 
        }
    } 

    
    if (NewWeapon == EWeaponType::Rifle && CurrentLevel < 2)
    {  
        UE_LOG(LogTemp, Warning, TEXT("라이플은 레벨 2부터 사용 가능 (현재 레벨: %d)"), CurrentLevel);
        return; 
        
    }

    if (NewWeapon == EWeaponType::Sniper && CurrentLevel < 3)
    {  
        UE_LOG(LogTemp, Warning, TEXT("[스나이퍼는 레벨 3부터 사용 가능 (현재 레벨: %d)"), CurrentLevel);
        return; 
       
    }
    


    // 기존 로직 
    CurrentWeapon = NewWeapon;

    switch (CurrentWeapon)
    {
    case EWeaponType::Pistol:
        MaxAmmo = 15;
        AttackDamage = 15.f;
        AttackRange = 3000.f;
        break;
    case EWeaponType::Rifle:
        MaxAmmo = 30;
        AttackDamage = 25.f;
        AttackRange = 5000.f;
        break;
    case EWeaponType::Sniper:
        MaxAmmo = 5;
        AttackDamage = 100.f;
        AttackRange = 10000.f;
        break;
    }
    
    CurrentAmmo = MaxAmmo; 
    
    UE_LOG(LogTemp, Warning, TEXT("[Combat] 무기 교체됨! 현재 무기 탄약: %d, 데미지: %f"), CurrentAmmo, AttackDamage);
}

// 찬우님 요청 [추가] #88
void UPFCombatComponent::AddAmmo(EWeaponType WeaponType, int32 Amount)
{
    
    if (CurrentWeapon == WeaponType)
    {
        
        CurrentAmmo = FMath::Clamp(CurrentAmmo + Amount, 0, MaxAmmo);

        UE_LOG(LogTemp, Warning, TEXT("[Combat] 탄약 획득! 현재 무기 탄약: %d / %d"), CurrentAmmo, MaxAmmo);
        
        // 나중에 UI에서 총알 정보(남은 총알 갯수) 출력하는 함수 넣어달라하면 여기에서 호출
    }
}

void UPFCombatComponent::PlayThrowAnimation()
{
    if (GrenadeCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] 수류탄이 없습니다!"));
        return;
    }

    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);
    
    if (Character && ThrowMontage)
    {
        // 던지기 몽타주가 시작되는 프레임에 손에 쥔 가짜 수류탄을 보이게 활성화합니다.
        if (HandGrenadeMesh)
        {
            HandGrenadeMesh->SetVisibility(true);
        }

        // 몽타주 재생 
        Character->PlayAnimMontage(ThrowMontage);
    }
}

// 실제 수류탄이 스폰되어 날아가는 함수
void UPFCombatComponent::ExecuteThrow()
{
    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);

    if (!Character || !GrenadeClass || GrenadeCount <= 0) return;

   
    if (HandGrenadeMesh)
    {
        HandGrenadeMesh->SetVisibility(false);
    }

    GrenadeCount--;
    UE_LOG(LogTemp, Warning, TEXT("[Combat] 수류탄 투척! 남은 개수: %d"), GrenadeCount);

    
    FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(HandSocketName); 
    
  
    FRotator SpawnRotation = Character->GetControlRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Character;

   
    GetWorld()->SpawnActor<APFGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, SpawnParams);
}

void UPFCombatComponent::FireWeapon()
{
    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] 총알이 부족하여 사격할 수 없습니다!"));
        return;
    }
    
    CurrentAmmo--;
    UE_LOG(LogTemp, Warning, TEXT("[Combat] 사격! 남은 총알: %d / %d"), CurrentAmmo, MaxAmmo);

    AActor* Owner = GetOwner();
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

    // 빨간선 디버그 (테스트 끝나면 지우세요)
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);

    if (bHit)
    {
       AActor* HitActor = HitResult.GetActor();
       if (HitActor)
       {
          APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
          if (Monster)
          {
             Monster->TakeDamageFromMonster(AttackDamage, Owner);
          }
       }
    }
}