// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingWeaponBase.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TX/TX.h"
#include "TX/Game/Abilities/ShootingAbilitySystemComponent.h"
#include "TX/Game/Abilities/ShootingGameplayAbility.h"
#include "TX/Game/Character/ShootingCharacterBase.h"
#include "TX/Game/Controller/ShootingPlayerController.h"


void AShootingWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingWeaponBase, OwningCharacter);

	// 在其他地方同步特效
	DOREPLIFETIME_CONDITION(AShootingWeaponBase, BurstCount, COND_SkipOwner);

	// 只有owner需要判断弹药
	DOREPLIFETIME_CONDITION(AShootingWeaponBase, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AShootingWeaponBase, CurrentAmmoInClip, COND_OwnerOnly);
}

// Sets default values
AShootingWeaponBase::AShootingWeaponBase()
	: OwningCharacter(nullptr)
	  , OwningAbilitySystemComponent(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	WeaponMesh1P->SetVisibility(false, true);
	WeaponMesh1P->SetupAttachment(RootComponent);
	WeaponMesh1P->SetCastShadow(false);
	WeaponMesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	WeaponMesh3P->SetupAttachment(RootComponent);
	WeaponMesh3P->SetCastShadow(true);
	WeaponMesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
}

USkeletalMeshComponent* AShootingWeaponBase::GetWeaponMesh()
{
	return OwningCharacter.IsValid() && OwningCharacter->IsFirstPerson() ? WeaponMesh1P : WeaponMesh3P;
}


void AShootingWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 初始填充弹匣
	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}
}


void AShootingWeaponBase::StartFire()
{
	// 服务器和客户端都要执行
	if (GetLocalRole() < ROLE_Authority)
	{
		StartFireOnServer();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AShootingWeaponBase::StopFire()
{
	if (GetLocalRole() < ROLE_Authority && OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
	{
		StopFireOnServer();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AShootingWeaponBase::StartReload(bool bFromReplication /*= false*/)
{
	// owner客户端执行请求
	if (!bFromReplication && GetLocalRole() < ROLE_Authority)
	{
		StartReloadOnServer();
	}

	// owner->服务器->其他客户端
	// 同步客户端或者服务器执行
	if (bFromReplication || CanReload())
	{
		// 想改变状态
		bPendingReload = true;
		DetermineWeaponState();

		// TODO 换弹动画
		// 如果没有动画，间隔时间用NoAnimReloadDuration
		float AnimDuration = WeaponConfig.NoAnimReloadDuration;
		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AShootingWeaponBase::StopReload,
		                                AnimDuration, false);
		if(GetLocalRole() == ROLE_Authority)
		{
			// 弹药的数值变化必须由服务器执行 在填弹动画前一点执行
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon,this,&AShootingWeaponBase::ReloadWeapon,AnimDuration - 0.1f,false);
		}

		// 播放本地音效
		if(OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AShootingWeaponBase::StopReload()
{
	// 出状态机，此时状态需要为RELOADING
	if(CurrentState == EWeaponState::RELOADING)
	{
		bPendingReload = false;
		DetermineWeaponState();
		// TODO 停止换弹动画
	}
}

void AShootingWeaponBase::ReloadWeapon()
{
	int32 AmmoToLoad = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if(HasInfiniteClip())
	{
		AmmoToLoad = WeaponConfig.AmmoPerClip - CurrentAmmoInClip;
	}

	if(AmmoToLoad > 0)
	{
		CurrentAmmoInClip += AmmoToLoad;
	}

	// if(HasInfiniteClip())
	// {
		// CurrentAmmo
	// }
}

bool AShootingWeaponBase::StartReloadOnServer_Validate()
{
	return true;
}

void AShootingWeaponBase::StartReloadOnServer_Implementation()
{
	StartReload();
}

bool AShootingWeaponBase::CanFire()
{
	bool bCanFire = OwningCharacter.IsValid() && OwningCharacter->CanFire();
	bool bStateOkToFire = (CurrentState == EWeaponState::IDLE) || (CurrentState == EWeaponState::FIRING);

	// 角色能够开火 && 武器状态允许开火 && 正在没有在填装
	return bCanFire && bStateOkToFire && !bPendingReload;
}

bool AShootingWeaponBase::CanReload()
{
	bool bCanReload = OwningCharacter.IsValid() && OwningCharacter->CanReload();
	// 弹匣不满 && 总弹药>弹匣(有备弹)
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteAmmo());
	bool bStateOkToReload = CurrentState == EWeaponState::IDLE || CurrentState == EWeaponState::FIRING;

	return bCanReload && bGotAmmo && bStateOkToReload;
}

void AShootingWeaponBase::AddAbilitiesToASC(UShootingAbilitySystemComponent* InAbilitySystemComponent)
{
	if (GetLocalRole() != ROLE_Authority || !InAbilitySystemComponent || !IsValid(InAbilitySystemComponent)) { return; }

	for (const TSubclassOf<UShootingGameplayAbility>& Ability : Abilities)
	{
		AbilityHandles.Add(
			InAbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->GetInputID()), this)));
	}
}

void AShootingWeaponBase::RemoveAbilitiesFromASC(UShootingAbilitySystemComponent* InAbilitySystemComponent)
{
	if (GetLocalRole() != ROLE_Authority || !InAbilitySystemComponent || !IsValid(InAbilitySystemComponent)) { return; }

	for (const FGameplayAbilitySpecHandle& AbilityHandle : AbilityHandles)
	{
		InAbilitySystemComponent->ClearAbility(AbilityHandle);
	}
}

void AShootingWeaponBase::SetOwningCharacter(AShootingCharacterBase* InCharacter)
{
	OwningCharacter = InCharacter;
	if (OwningCharacter.IsValid())
	{
		SetOwner(OwningCharacter.Get());
		AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

}

int32 AShootingWeaponBase::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AShootingWeaponBase::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AShootingWeaponBase::GetRemainAmmo() const
{
	return CurrentAmmo - CurrentAmmoInClip;
}

bool AShootingWeaponBase::HasInfiniteAmmo() const
{
	return WeaponConfig.bInfiniteAmmo;	
}

bool AShootingWeaponBase::HasInfiniteClip() const
{
	return WeaponConfig.bInfiniteClip;
}

void AShootingWeaponBase::Equip()
{
	if (OwningCharacter.IsValid())
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bIsEquipped = true;
		
		if(WeaponMesh3P)
		{
			WeaponMesh3P->AttachToComponent(OwningCharacter->Get3PMesh(),
			                                FAttachmentTransformRules::SnapToTargetIncludingScale,
			                                OwningCharacter->GetWeaponSocketName());
			WeaponMesh3P->SetRelativeRotation(Weapon1PRelativeRotation);
			WeaponMesh3P->SetRelativeLocation(Weapon1PRelativeLocation);

			if(OwningCharacter->IsFirstPerson())
			{
				WeaponMesh3P->SetVisibility(false, true);
			}
			else
			{
				WeaponMesh3P->SetVisibility(true, true);	
			}
		}

		if(WeaponMesh1P)
		{
			WeaponMesh1P->AttachToComponent(OwningCharacter->Get1PMesh(),
			                                FAttachmentTransformRules::SnapToTargetIncludingScale,
			                                OwningCharacter->GetWeaponSocketName());
			WeaponMesh1P->SetRelativeRotation(Weapon1PRelativeRotation);
			WeaponMesh1P->SetRelativeLocation(Weapon1PRelativeLocation);
			
			if(OwningCharacter->IsFirstPerson())
			{
				WeaponMesh1P->SetVisibility(true, true);
			}
			else
			{
				WeaponMesh1P->SetVisibility(false, true);	
			}
		}
	}
}

UTexture2D* AShootingWeaponBase::GetWeaponIcon() const
{
	return WeaponIcon;
}

void AShootingWeaponBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	// 已经有了就不能再捡了
	if (OwningCharacter.IsValid()) { return; }

	AShootingCharacterBase* InCharacter = Cast<AShootingCharacterBase>(OtherActor);
	if (IsValid(OtherActor) && InCharacter)
	{
		InCharacter->AddWeaponToInventory(this, false);
	}
}

// Called when the game starts or when spawned
void AShootingWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

bool AShootingWeaponBase::StartFireOnServer_Validate()
{
	return true;
}

void AShootingWeaponBase::StartFireOnServer_Implementation()
{
	StartFire();
}

void AShootingWeaponBase::StopFireOnServer_Implementation()
{
	StopFire();
}

bool AShootingWeaponBase::StopFireOnServer_Validate()
{
	return true;
}

void AShootingWeaponBase::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::IDLE;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if(!CanReload())
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::RELOADING;
			}
		}
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState::FIRING;
		}
	}
	// 正在切换武器
	else if (bPendingEquip)
	{
	}

	SetWeaponState(NewState);
}

void AShootingWeaponBase::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	// 如果之前是开火状态，现在不是了，那么就是开火结束
	if (PrevState == EWeaponState::FIRING && NewState != EWeaponState::FIRING)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	// 之前没开火，开始开火
	if (PrevState != EWeaponState::FIRING && NewState == EWeaponState::FIRING)
	{
		OnBurstStarted();
	}
}

UAudioComponent* AShootingWeaponBase::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = nullptr;
	if(Sound && OwningCharacter.IsValid())
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, OwningCharacter->GetRootComponent());
	}

	return AC;
}

void AShootingWeaponBase::OnBurstFinished()
{
	// 结束设计状态清除还在等待的发射
	GetWorldTimerManager().ClearTimer(TimerHandle_HandelFiring);

	// 停止特效
	StopStimulateWaeponFire();
	BurstCount = 0;

	
	bRefiring = false;
}

void AShootingWeaponBase::OnBurstStarted()
{
	const float GameTime = GetWorld()->GetTimeSeconds();

	// 开火间隔控制
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandelFiring, this, &AShootingWeaponBase::HandleFiring,
		                                LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AShootingWeaponBase::OnRep_Reload()
{
	if(bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

void AShootingWeaponBase::UseAmmo()
{
	if(!HasInfiniteAmmo())
	{
		CurrentAmmoInClip--;
	}
	if(!HasInfiniteClip() && !HasInfiniteAmmo())
	{
		CurrentAmmo--;
	}

	UE_LOG(LogTemp, Warning, TEXT("Ammo %d/%d"), CurrentAmmoInClip, CurrentAmmo);
}

void AShootingWeaponBase::HandleFiring()
{
	// 本地先开火再请求服务器 本地的判定决定特效显示，伤害死亡积分只能由服务器决定
	if (CurrentAmmoInClip > 0 || HasInfiniteAmmo() && CanFire())
	{
		// 在非服务器上播放FX
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		// 玩家客户端进行射击
		if (OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
		{
			// 开火 消耗弹药
			FireWeapon();

			UseAmmo();

			// 放这里加的话 服务器加不到?
			BurstCount++;
		}
	}
	// 弹匣空 有备弹
	else if(CanReload())
	{
		StartReload();
	}
	// 没子弹 local播放音效和UI提示
	else if(OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
	{
		// 连发状态不提示 松开鼠标再按提示
		if(GetCurrentAmmo() == 0 && !bRefiring)
		{
				
		}

		if(BurstCount > 0)
		{
			OnBurstFinished();
		}
	}
	else
	{
		OnBurstFinished();	
	}

	if (OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
	{
		// 服务器执行消耗弹药
		if(GetLocalRole() < ROLE_Authority)
		{
			// 服务器加bursrcount在这里
			HandleFiringOnServer();
		}

		// 连发，只要鼠标不松开就不会设置State回来,接下来满足间隔就能开火
		bRefiring = CurrentState == EWeaponState::FIRING && WeaponConfig.TimeBetweenShots > 0.0f;
		if (bRefiring)
		{
			// TODO 时间间隔考虑帧率修正?
			GetWorldTimerManager().SetTimer(TimerHandle_HandelFiring, this, &AShootingWeaponBase::HandleReFiring,
			                                WeaponConfig.TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AShootingWeaponBase::HandleFiringOnServer_Implementation()
{
	// 状态能够开火 子弹>0
	// TODO AMMO > 0
	const bool bShouldUpdateAmmo = CurrentAmmoInClip > 0 && CanFire();

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// 消耗弹药
		UseAmmo();

		//
		BurstCount++;
	}
}

bool AShootingWeaponBase::HandleFiringOnServer_Validate()
{
	return true;
}

void AShootingWeaponBase::HandleReFiring()
{
	// Update TimerIntervalAdjustment
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	// if (bAllowAutomaticWeaponCatchup)
	// {
	// TimerIntervalAdjustment -= SlackTimeThisFrame;
	// }

	HandleFiring();
}

void AShootingWeaponBase::FireWeapon()
{
}

void AShootingWeaponBase::SimulateWeaponFire()
{
	// 服务器不需要播放特效
	if (GetLocalRole() == ROLE_Authority && CurrentState != FIRING)
	{
		return;
	}

	// 视角不同效果不同?
	// 1.焰火
	if (MuzzleFX)
	{
		// 第一人称控制器要生成两组特效
		// 非第一人称控制器使用get到的mesh?
		USkeletalMeshComponent* UseWaeponMesh = GetWeaponMesh();
		if (MuzzlePSC == nullptr)
		{
			if (OwningCharacter.IsValid() && OwningCharacter->IsLocallyControlled())
			{
				AController* Controller = OwningCharacter->GetController();
				if (Controller)
				{
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh1P, MuzzleAttachPoint);
					MuzzlePSC->SetOwnerNoSee(false);
					MuzzlePSC->SetOnlyOwnerSee(true);

					MuzzlePSC_3P = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh3P, MuzzleAttachPoint);
					MuzzlePSC_3P->SetOwnerNoSee(true);
					MuzzlePSC_3P->SetOnlyOwnerSee(false);
				}
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWaeponMesh, MuzzleAttachPoint);
			}
		}
	}

	// 2.动画

	// 3.声音
	PlayWeaponSound(FireSound);

	// 4.相机抖动
}

void AShootingWeaponBase::StopStimulateWaeponFire()
{
}

FVector AShootingWeaponBase::GetAdjustedAim() const
{
	AShootingPlayerController* const PlayerController = OwningCharacter.IsValid()
		                                                    ? Cast<AShootingPlayerController>(
			                                                    OwningCharacter->GetController())
		                                                    : nullptr;
	FVector FinalAim = FVector::ZeroVector;
	// 玩家
	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	// TODO AI

	return FinalAim;
}

FVector AShootingWeaponBase::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AShootingPlayerController* PC = OwningCharacter.IsValid()
		                                ? Cast<AShootingPlayerController>(OwningCharacter->GetController())
		                                : nullptr;

	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		OutStartTrace = OutStartTrace + AimDir * ((OwningCharacter->GetActorLocation() - OutStartTrace) | AimDir);
	}

	// TODO AI

	return OutStartTrace;
}

FHitResult AShootingWeaponBase::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, OwningCharacter.Get());
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(OwningCharacter.Get());

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 2.0f, 0, 1.0f);

	return Hit;
}

void AShootingWeaponBase::OnRep_BurstCount()
{
	if(BurstCount > 0)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Sim"));
		SimulateWeaponFire();		
	}
	else
	{
		
	}
}

float AShootingWeaponBase::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;

	if(OwningCharacter.IsValid())
	{
		UAnimMontage* UseAnim = OwningCharacter->IsFirstPerson() ? Animation.Anim1P : Animation.Anim3P;
		if(UseAnim)
		{
			Duration = OwningCharacter->PlayAnimMontage(UseAnim);
		}	
	}

	return Duration;
}

FWeaponAnim AShootingWeaponBase::GetEquipAnim() const
{
	return EquipAnim;
}


// Called every frame
void AShootingWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// if(GetLocalRole() == ROLE_SimulatedProxy)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("BurstCount %d"), BurstCount);
	// }
}
