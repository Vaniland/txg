// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShootingWeaponBase.generated.h"

class USoundCue;
class UCapsuleComponent;
class AShootingCharacterBase;
class UShootingGameplayAbility;
struct FGameplayAbilitySpecHandle;
class UGameplayAbility;
class UShootingAbilitySystemComponent;

UENUM(BlueprintType)
enum EWeaponState
{
	IDLE,
	FIRING,
	RELOADING,
	EQUIPPING,
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	bool bInfiniteAmmo;

	UPROPERTY(EditDefaultsOnly)
	bool bInfiniteClip;

	// 弹药总上限
	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;

	// 弹匣容量
	UPROPERTY(EditDefaultsOnly)
	int32 AmmoPerClip;

	// 初始弹匣数量
	UPROPERTY(EditDefaultsOnly)
	int32 InitialClips;

	// 开火间隔
	UPROPERTY(EditDefaultsOnly)
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly)
	float NoAnimReloadDuration;
	
	FWeaponData()
		: bInfiniteAmmo(false)
		  , bInfiniteClip(false)
		  , MaxAmmo(0)
		  , AmmoPerClip(30)
		  , InitialClips(2)
		  , TimeBetweenShots(0.2f)
		  , NoAnimReloadDuration(2.0f)
	{
	}
	
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Anim1P;
	
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Anim3P;

	FWeaponAnim()
		: Anim1P(nullptr)
		  , Anim3P(nullptr)
	{
	}
	
};


UCLASS()
class TX_API AShootingWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Sets default values for this actor's properties
	AShootingWeaponBase();

	// 1p or 3p
	USkeletalMeshComponent* GetWeaponMesh();

	virtual void PostInitializeComponents() override;

	// 
	virtual void StartFire();
	virtual void StopFire();

	//
	virtual void StartReload(bool bFromReplication = false);

	virtual void StopReload();

	virtual void ReloadWeapon();	

	UFUNCTION(Reliable, Server, WithValidation)
	void StartReloadOnServer();
	bool StartReloadOnServer_Validate();
	void StartReloadOnServer_Implementation();

	bool CanFire();

	bool CanReload();

	

	bool IsInfinitAmmo() const {return WeaponConfig.bInfiniteAmmo;}

	void AddAbilitiesToASC(UShootingAbilitySystemComponent* InAbilitySystemComponent);

	void RemoveAbilitiesFromASC(UShootingAbilitySystemComponent* InAbilitySystemComponent);

	void SetOwningCharacter(AShootingCharacterBase* InCharacter);

	int32 GetCurrentAmmo() const;
	int32 GetCurrentAmmoInClip() const;
	int32 GetRemainAmmo() const;

	bool HasInfiniteAmmo() const;

	bool HasInfiniteClip() const;

	void Equip();

	UTexture2D* GetWeaponIcon() const;
	
	FWeaponAnim GetEquipAnim() const;
private:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Reliable, Server, WithValidation)
	void StartFireOnServer();
	virtual bool StartFireOnServer_Validate();
	virtual void StartFireOnServer_Implementation();
	
	UFUNCTION(Reliable, Server, WithValidation)
	void StopFireOnServer();
	virtual bool StopFireOnServer_Validate();
	virtual void StopFireOnServer_Implementation();

	void DetermineWeaponState();

	void SetWeaponState(EWeaponState NewState);

	UAudioComponent* PlayWeaponSound(USoundCue* Sound);


	// 武器结束开火
	virtual void OnBurstFinished();

	// 武器开始开火
	virtual void OnBurstStarted();

	UFUNCTION()
	void OnRep_Reload();

	void UseAmmo();

	void HandleFiring();

	UFUNCTION(Reliable, Server, WithValidation)
	void HandleFiringOnServer();
	void HandleFiringOnServer_Implementation();
	bool HandleFiringOnServer_Validate();

	void HandleReFiring();

	virtual void FireWeapon();

	// 开火特效?
	void SimulateWeaponFire();

	void StopStimulateWaeponFire();

	virtual FVector GetAdjustedAim() const;
	
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	UFUNCTION()
	void OnRep_BurstCount();

	// Animation
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

protected:
	UPROPERTY(BlueprintReadOnly, Replicated , Category = "Shooter|Weapon")
	TWeakObjectPtr<AShootingCharacterBase> OwningCharacter;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Shooter|Ability")
	TWeakObjectPtr<UShootingAbilitySystemComponent> OwningAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Shooter|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh3P;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Shooter|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh1P;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Ability", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UShootingGameplayAbility>> Abilities;
	
	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Ability")
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	// VFX 枪口火焰
	UPROPERTY(Transient)
	TObjectPtr<UParticleSystemComponent> MuzzlePSC;
	
	UPROPERTY(Transient)
	TObjectPtr<UParticleSystemComponent> MuzzlePSC_3P;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Effects")
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Sound")
	USoundCue* FireSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Sound")
	USoundCue* ReloadSound;

	//
	UPROPERTY(EditDefaultsOnly)
	FVector Weapon1PRelativeLocation;
	UPROPERTY(EditDefaultsOnly)
	FRotator Weapon1PRelativeRotation;

	//
	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<EWeaponState> CurrentState;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* WeaponIcon;

	UPROPERTY(EditDefaultsOnly, Category= "Shooter|Config")
	FWeaponData WeaponConfig;

	// 当前弹药量
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	// 当前弹匣中的弹药数
	UPROPERTY(Transient,Replicated)
	int32 CurrentAmmoInClip;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCount)
	int32 BurstCount;

	float LastFireTime;

	// States convert
	bool bWantsToFire;
	
	bool bIsEquipped;
	
	bool bPendingEquip;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	//
	bool bRefiring;

	// Animations
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Animation")
	FWeaponAnim EquipAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Animation")
	FWeaponAnim FireAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Animation")
	FWeaponAnim ReloadAnim;

protected:
	FTimerHandle TimerHandle_HandelFiring;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;
	FTimerHandle TimerHandle_OnEquipFinished;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
