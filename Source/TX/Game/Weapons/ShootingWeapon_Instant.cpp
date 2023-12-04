// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingWeapon_Instant.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TX/TX.h"
#include "TX/Game/Character/ShootingCharacterBase.h"


float AShootingWeapon_Instant::GetCurrentSpread() const
{
	float FinalSpread = InstantConfig.WeaponSpread + CurrentFiringSpread;
	// 瞄准减少扩散
	if (OwningCharacter.IsValid() && OwningCharacter->IsTargeting())
	{
		FinalSpread *= InstantConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

// Sets default values
AShootingWeapon_Instant::AShootingWeapon_Instant()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AShootingWeapon_Instant::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShootingWeapon_Instant::FireWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire"));
	// 随机扩散
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;

	// 处理碰撞
	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	// 扩散不会大于最大值
	CurrentFiringSpread = FMath::Min(InstantConfig.FiringSpreadMax, CurrentFiringSpread + InstantConfig.FiringSpreadIncrement);
}

void AShootingWeapon_Instant::OnBurstFinished()
{
	Super::OnBurstFinished();

}

void AShootingWeapon_Instant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin,
                                                const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// local是第一人称模型
	if(OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// 打中有控制的才会反应?
		if(Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// 服务器验证是否击中 服务器验证不中会拒绝
			NotifyHitOnServer(Impact, ShootDir, RandomSeed, ReticleSpread);
		}
		// 没打中人
		else if(Impact.GetActor() == nullptr)
		{
			if(Impact.bBlockingHit)
			{
				
			}
			else
			{
				
			}
		}
	}

	// Client
	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

void AShootingWeapon_Instant::NotifyHitOnServer_Implementation(const FHitResult Impact,
	FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// 如果服务器没通过验证，客户端数据会同步回去?
	ProcessInstantHit_Confirmed(Impact, GetActorLocation(), ShootDir, RandomSeed, ReticleSpread);
}

bool AShootingWeapon_Instant::NotifyHitOnServer_Validate(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir,
	int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AShootingWeapon_Instant::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin,
	const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// 伤害数据处理 只有服务器处才能处理伤害 判定死亡
	if(GetNetMode() != NM_Client || GetLocalRole() == ROLE_Authority)
	{
		if(Impact.GetActor())
		{
			float ActualHitDamage = InstantConfig.HitDamage;
			switch (Impact.PhysMaterial.Get()->SurfaceType)
			{
			case SURFACE_HEAD:
				ActualHitDamage *= 5.f;
				break;
				
			default:
				break;
			}
			
			// Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->GetController(), this);
			UGameplayStatics::ApplyPointDamage(Impact.GetActor(), ActualHitDamage, ShootDir, Impact,
			                                   OwningCharacter->GetController(), this, InstantConfig.DamageType);
			// UE_LOG(LogTemp, Warning, TEXT("Damage %s"), *Impact.GetActor()->GetName());
		}
	}

	// ??
	if(GetLocalRole() == ROLE_Authority)
	{
		
	}

	// 生成弹道 击中效果
	if(GetNetMode() != NM_DedicatedServer)
	{
	}
}

// Called every frame
void AShootingWeapon_Instant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(bIsEquipped)
	{
		if(CurrentState != FIRING && CurrentFiringSpread > 0.f)
		{
			// CurrentFiringSpread = FMath::Max(InstantConfig.WeaponSpread, CurrentFiringSpread - InstantConfig.FiringSpreadDecrease * DeltaTime);
			CurrentFiringSpread = FMath::Clamp(CurrentFiringSpread - InstantConfig.FiringSpreadDecrease * DeltaTime,
			                                   0, InstantConfig.FiringSpreadMax);
		}
	}
}

