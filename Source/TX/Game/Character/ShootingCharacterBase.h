// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShootingCharacterBase.generated.h"

class USoundCue;
enum EWeaponType : uint8;
class UGameplayAbilitySet;
class UShootingGameplayAbility;
class UGameplayAbility;
struct FGameplayAbilitySpecHandle;
class UShootingAbilitySystemComponent;
class AShootingWeaponBase;
class USpringArmComponent;
class UCameraComponent;

// DECLARE_DELEGATE_OneParam(FOnWeaponEquippedDelegate, UTexture2D*)

USTRUCT(BlueprintType)
struct FShootingInventory
{
	GENERATED_BODY()

	// Add UPROPERTY() for rep

	UPROPERTY()
	TWeakObjectPtr<AShootingWeaponBase> PrimaryWeapon;

	UPROPERTY()
	TWeakObjectPtr<AShootingWeaponBase> SecondaryWeapon;
};

UCLASS()
class TX_API AShootingCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShootingCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void PawnClientRestart() override;

	virtual void PossessedBy(AController* NewController) override;


	FText GetAmmoText() const;
	float GetHealthPercent() const;
	UTexture2D* GetWeaponIcon() const;

	bool IsDead() const { return bIsDying; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddAbility(TSubclassOf<UShootingGameplayAbility> Ability);

	void EquipWeapon(AShootingWeaponBase* NewWeapon);

	UFUNCTION(Reliable, Server)
	void EquipWeaponOnServer(AShootingWeaponBase* NewWeapon);
	void EquipWeaponOnServer_Implementation(AShootingWeaponBase* NewWeapon);

	void SetCurrentWeapon(AShootingWeaponBase* NewWeapon, AShootingWeaponBase* LastWeapon);

	bool CanFire() const;

	bool CanReload() const;

	bool IsTargeting() const;

	UFUNCTION(BlueprintCallable)
	bool IsSprinting() const;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	USkeletalMeshComponent* GetPawnMesh() const;

	// Animation
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;
	virtual void StopAnimMontage(UAnimMontage* AnimMontage) override;

private:
	// bool DoesWeaponExist(AShootingWeaponBase* InWeapon) const;
private:
	// CharacterMovement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);

	void BeginCrouch();

	void EndCrouch();

	void PlayHit(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	
	UFUNCTION(Reliable,NetMulticast)
	void PlayHitMulticast(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	virtual void PlayHitMulticast_Implementation(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	
	UFUNCTION(Reliable,NetMulticast)
	void PlayDeadMulticast(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser, bool bHeadShot = false);
	virtual void PlayDeadMulticast_Implementation(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser, bool bHeadShot = false);

	
	// Weapon
	void StartWeaponFire();

	// Player pressed start fire action
	void OnStartFire();
	
	void OnStopWaeponFire();

	// Sprint
	void OnStartSprint();
	void OnStopSprint();

	//
	void OnEquipPrimaryWeapon();
	void OnEquipSecondaryWeapon();

	// reload
	void OnStartReload();

	// DropWeapon
	void OnDropWeapon();
	UFUNCTION(Reliable, Server)
	void DropWeaponOnServer();
	void DropWeaponOnServer_Implementation();

	UFUNCTION(Reliable, NetMulticast)
	void DropWeaponMulticast();
	void DropWeaponMulticast_Implementation();

	void SetRunning(bool bNewRunning);

	UFUNCTION(Reliable, Server)
	void SetRunningOnServer(bool bNewRunning);
	void SetRunningOnServer_Implementation(bool bNewRunning);
	bool SetRunningOnServer_Validate(bool bNewRunning);

	UFUNCTION(Reliable, NetMulticast)
	void PickupWeaponMulticast(AShootingWeaponBase* NewWeapon);
	void PickupWeaponMulticast_Implementation(AShootingWeaponBase* NewWeapon);


	// 如果是服务器判定捡到武器装备上来，要同步到客户端
	UFUNCTION()
	void OnRep_CurrentWeapon(AShootingWeaponBase* LastWeapon);
	
	UFUNCTION()
	void OnRep_Inventory();

	// UFUNCTION()
	// void OnRep_HitRepCounter();

private:
	bool bASCInputBound;

	bool bInteractable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon",meta = (AllowPrivateAccess = "true"))
	FName WeaponSocketName;
public:
	FGameplayAbilitySpecHandle AddAbility(TSubclassOf<UGameplayAbility> InAbility);
	
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Inventory")
	bool AddWeaponToInventory(AShootingWeaponBase* NewWeapon, bool bEquipWeapon = false);

	UFUNCTION(BlueprintCallable)
	AShootingWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable)
	EWeaponType GetCurrentWeaponType() const;

	USkeletalMeshComponent* Get3PMesh() const { return GetMesh(); }
	USkeletalMeshComponent* Get1PMesh() const { return Mesh1P; }
	FName GetWeaponSocketName() const { return WeaponSocketName; }

	bool IsFirstPerson() const;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category=Health)
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,  Category=Health)
	bool bIsDying;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<UCameraComponent> Camera1P;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Camera")
	// TObjectPtr<UCameraComponent> Camera3P;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Camera")
	// TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(Transient, VisibleAnywhere,ReplicatedUsing = OnRep_CurrentWeapon, Category = "Shooter|Weapon")
	TObjectPtr<AShootingWeaponBase> CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Shooter|Ability")
	TObjectPtr<UShootingAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Ability")
	TArray<TSubclassOf<UShootingGameplayAbility>> DefaultAbilities;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Inventory,Category = "Shooter|Inventory")
	FShootingInventory Inventory;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* DeathAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Sound")
	USoundCue* PickupWeaponSound;

	FTimerHandle TimerHandle_Interact;

	// UPROPERTY(Transient, ReplicatedUsing = OnRep_HitRepCounter)
	// uint8 HitRepCounter;
	
	bool bWantsToRun;
	
	bool bWantsToFire;

	bool bIsTargeting;

	void BindASCInput();
	
};


