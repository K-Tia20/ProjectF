#include "Weapon/PFGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "NiagaraComponent.h" 
#include "DrawDebugHelpers.h"
#include "Monster/PFMonsterCharacter.h"


APFGrenade::APFGrenade()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = CollisionComponent;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(RootComponent);
	
	ExplosionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionSphereComponent"));
	ExplosionSphereComponent->SetupAttachment(RootComponent);
	ExplosionSphereComponent->InitSphereRadius(400.0f);
	ExplosionSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionSphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	ExplosionEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplosionEffect"));
	ExplosionEffect->SetupAttachment(RootComponent);
	ExplosionEffect->bAutoActivate = false;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
}


void APFGrenade::BeginPlay()
{
	Super::BeginPlay();
	// 타이머 설정
	GetWorldTimerManager().SetTimer(ExplodeTimerHandle, this, &APFGrenade::Explode, FuseTime, false);
}



void APFGrenade::Explode()
{
	FVector ExplodeLocation = GetActorLocation();

	// 나이아가라 이펙트 on(활성화)
	if (ExplosionEffect)
	{
		ExplosionEffect->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		ExplosionEffect->Activate(true);
	}

	// 청각 효과 (사운드 재생)
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, ExplodeLocation);
	}

	if (ExplosionSphereComponent)
	{
		TArray<AActor*> OverlappingActors;
        
		//  ExplosionSphereComponent 이랑 overlap된 액터 탐색
		ExplosionSphereComponent->GetOverlappingActors(OverlappingActors);

		for (AActor* HitActor : OverlappingActors)
		{
			if (HitActor)
			{
				// 몬스터만 피격
				APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
				if (Monster)
				{
					Monster->TakeDamageFromMonster(ExplosionDamage, GetOwner());
                    
					UE_LOG(LogTemp, Log, TEXT("[Grenade] %s 몬스터에게 수류탄 데미지 %f 적용 완료!"), 
						*Monster->GetName(), ExplosionDamage);
				}
			}
		}
	}

	// 이후 처리
	if (GrenadeMesh)
	{
		GrenadeMesh->SetHiddenInGame(true);
	}
	
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, [this]()
		{this->Destroy();}, 5.0f, false);
}