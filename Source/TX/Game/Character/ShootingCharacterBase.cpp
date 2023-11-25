// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacterBase.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "TX/ShootingGameType.h"
#include "TX/Game/ShootingGameMode.h"
#include "TX/Game/Abilities/ShootingAbilitySystemComponent.h"
#include "TX/Game/Abilities/ShootingGameplayAbility.h"
#include "TX/Game/Components/ShootingCharacterMovementComponent.h"
#include "TX/Game/Controller/ShootingPlayerController.h"
#include "TX/Game/Weapons/ShootingWeaponBase.h"


// Sets default values
AShootingCharacterBase::AShootingCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShootingCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
	  , bASCInputBound(false)
	  , WeaponSocketName(NAME_None)
	  , Health(100)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	
	// SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	// SpringArmComponent->SetupAttachment(RootComponent);

	// Camera3P = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera3P"));
	// Camera3P->SetupAttachment(SpringArmComponent);
	// Camera3P->bUsePawnControlRotation = true;

	Camera1P = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera1P"));
	Camera1P->SetupAttachment( GetRootComponent());
	Camera1P->bUsePawnControlRotation = true;

	// 1Pmesh不参与任何碰撞
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(Camera1P);
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetOwnerNoSee(false);
	Mesh1P->SetCastShadow(false);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionProfileName(FName(TEXT("NoCollision")));

	// 3P模型别人可见，view actor不可见
	GetMesh()->SetCastShadow(true);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetOwnerNoSee(true);


	AbilitySystemComponent = CreateDefaultSubobject<UShootingAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// Minmal复制tag,attribute 不复制GE
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

}

void AShootingCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();

	UE_LOG(LogTemp,Warning,TEXT("%s PawnClientRestart"),*GetName());
}

void AShootingCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UE_LOG(LogTemp, Warning, TEXT("%s possessed by %s"), *GetName(), *NewController->GetName());
}

FText AShootingCharacterBase::GetAmmoText() const
{
	if(CurrentWeapon)
	{
		return FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentWeapon->GetCurrentAmmoInClip(),
		                                         CurrentWeapon->GetRemainAmmo()));
	}
	else
	{
		return FText::FromString(TEXT("0/0"));
	}
}

float AShootingCharacterBase::GetHealthPercent() const
{
	return Health / 100.0f;	
}

UTexture2D* AShootingCharacterBase::GetWeaponIcon() const
{
	return CurrentWeapon ? CurrentWeapon->GetWeaponIcon() : nullptr;
}

// Called when the game starts or when spawned
void AShootingCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	for(auto Ability : DefaultAbilities)
	{
		AddAbility(Ability);
	}
		
}

// Called every frame
void AShootingCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AShootingCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AShootingCharacterBase::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AShootingCharacterBase::Turn);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShootingCharacterBase::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShootingCharacterBase::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &AShootingCharacterBase::OnStartFire);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Released, this, &AShootingCharacterBase::OnStopWaeponFire);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AShootingCharacterBase::OnStartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AShootingCharacterBase::OnStopSprint);

	// BindASCInput();
}

void AShootingCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingCharacterBase, Health);
	DOREPLIFETIME(AShootingCharacterBase, CurrentWeapon);

	// 对于一般模式，仅仅同步给拥有者，因为换武器仅由拥有者决定
	DOREPLIFETIME_CONDITION(AShootingCharacterBase, Inventory, COND_OwnerOnly);
}

void AShootingCharacterBase::SetRunningOnServer_Implementation(bool bNewRunning)
{
	SetRunning(bNewRunning);
}

bool AShootingCharacterBase::SetRunningOnServer_Validate(bool bNewRunning)
{
	return true;
}

void AShootingCharacterBase::OnRep_CurrentWeapon(AShootingWeaponBase* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);	
}

void AShootingCharacterBase::OnRep_Inventory()
{
}

void AShootingCharacterBase::AddAbility(TSubclassOf<UShootingGameplayAbility> Ability)
{
	if(AbilitySystemComponent)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->GetInputID()), this));
	}
}

void AShootingCharacterBase::EquipWeapon(AShootingWeaponBase* NewWeapon)
{
	// 客户端切换提出请求
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
	// 服务器处装备武器，估计是地上捡到，需要在OnRep同步回去
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void AShootingCharacterBase::ServerEquipWeapon_Implementation(AShootingWeaponBase* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

void AShootingCharacterBase::SetCurrentWeapon(AShootingWeaponBase* NewWeapon, AShootingWeaponBase* LastWeapon)
{
	if (NewWeapon == LastWeapon) { return; }

	// 取消当前武器技能 再卸载

	//
	if(NewWeapon)
	{
		// UI PlayerController

		// OnRep执行下面的
		CurrentWeapon = NewWeapon;
		// ??
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->Equip();
		// 播放动画
		UAnimMontage* AnimToUse = IsFirstPerson() ? CurrentWeapon->GetEquipAnim().Anim1P : CurrentWeapon->GetEquipAnim().Anim3P;
		PlayAnimMontage(AnimToUse);
		
		// UI
		if(AShootingPlayerController* ShootingPC = GetController<AShootingPlayerController>())
		{
			ShootingPC->OnWeaponEquippedDelegate.ExecuteIfBound(CurrentWeapon->GetWeaponIcon());	
		}
		// CurrentWeapon->SetOwningCharacter(this);
	}
	
}


bool AShootingCharacterBase::DoesWeaponExist(AShootingWeaponBase* InWeapon) const
{
	for (const AShootingWeaponBase* Weapon : Inventory.Weapons)
	{
		if(Weapon->StaticClass() == InWeapon->StaticClass())
		{
			return true;
		}
	}

	return false;
}

void AShootingCharacterBase::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void AShootingCharacterBase::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void AShootingCharacterBase::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AShootingCharacterBase::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AShootingCharacterBase::BeginCrouch()
{
	Crouch();
}

void AShootingCharacterBase::EndCrouch()
{
	UnCrouch();
}

void AShootingCharacterBase::StartWeaponFire()
{
	if(!bWantsToFire)
	{
		bWantsToFire = true;
		if(CurrentWeapon)
		{
			CurrentWeapon->StartFire();	
		}
	}
}

void AShootingCharacterBase::OnStartFire()
{
	AShootingPlayerController* PC = Cast<AShootingPlayerController>(Controller);
	if(PC && PC->IsGameInputAllowed())
	{
		// 如果在跑步 停下来 开枪
		StartWeaponFire();
	}
}

void AShootingCharacterBase::OnStopWaeponFire()
{
	if(bWantsToFire)
	{
		bWantsToFire = false;
		if(CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}

void AShootingCharacterBase::OnStartSprint()
{
	AShootingPlayerController* PC = Cast<AShootingPlayerController>(Controller);
	if(PC && PC->IsGameInputAllowed())
	{
		// 跑步不能和开火一起
		OnStopWaeponFire();
		SetRunning(true);
	}
}

void AShootingCharacterBase::OnStopSprint()
{
	SetRunning(false);
}


void AShootingCharacterBase::SetRunning(bool bNewRunning)
{
	bWantsToRun = bNewRunning;

	if(GetLocalRole() < ROLE_Authority)
	{
		SetRunningOnServer(bNewRunning);
	}
}


bool AShootingCharacterBase::CanFire() const
{
	return true;
}

bool AShootingCharacterBase::CanReload() const
{
	return true;
}

bool AShootingCharacterBase::IsTargeting() const
{
	return bIsTargeting;
}

bool AShootingCharacterBase::IsSprinting() const
{
	if(!GetCharacterMovement())
	{
		return false;
	}

	// more?
	return bWantsToRun;
}

float AShootingCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                         AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if(ActualDamage > 0.0f)
	{
		Health -= ActualDamage;
		if(Health <= 0.0f)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			// TODO 攻击者和受击者的HUD效果
			
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("%s Health:%f"), *GetName(), Health);

	return ActualDamage;
}

void AShootingCharacterBase::OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator,
	AActor* DamageCauser)
{
	if(bIsDying)
	{
		return;
	}
}


bool AShootingCharacterBase::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer,
	AActor* DamageCauser)
{
	if(!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	// 不能超过下限0
	Health = FMath::Min(0.0f, Health);

	UE_LOG(LogTemp,Warning,TEXT("%s Die"),*GetName());

	bIsDying = true;
	
	// TODO 如果是环境击杀 用之前造成伤害的player加分
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass
		                                      ? DamageEvent.DamageTypeClass.GetDefaultObject()
		                                      : GetDefault<UDamageType>();
	
	
	// GameMode 算分 算完分之后才可以剥离控制器
	AShootingGameMode* ShootingGameMode = Cast<AShootingGameMode>(GetWorld()->GetAuthGameMode());
	ShootingGameMode->Killed(Killer, GetController(), this, DamageType);

	// 死亡相关 动画，音效，
	DetachFromControllerPendingDestroy();
	// OnDeath(ActualDamage, DamageEvent, Killer);
	{
		SetLifeSpan(5.f);
		
		GetCapsuleComponent()->SetCollisionEnabled((ECollisionEnabled::NoCollision));
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	}

	return true;
}

bool AShootingCharacterBase::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer,
	AActor* DamageCauser) const
{
	// 已经死了 已经被销毁 非服务器不可判定死亡
	// Level变化?
	if(bIsDying
		|| !IsValid(this)
		|| GetLocalRole() != ROLE_Authority
		// || GetWorld()->GetAuthGameMode() == nullptr
		// || 
		)
	{
		return false;	
	}

	return true;
}

USkeletalMeshComponent* AShootingCharacterBase::GetPawnMesh() const
{
	return IsValid(this) && IsFirstPerson() ? Mesh1P : GetMesh();
}

float AShootingCharacterBase::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if(AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		if(UseMesh->GetAnimInstance())
		{
			return UseMesh->GetAnimInstance()->Montage_Play(AnimMontage, InPlayRate);
		}
	}
	
	return 0.f;
}


FGameplayAbilitySpecHandle AShootingCharacterBase::AddAbility(TSubclassOf<UGameplayAbility> InAbility)
{
	if(AbilitySystemComponent)
	{
		return {};
	}
	
	return {};
}

bool AShootingCharacterBase::AddWeaponToInventory(AShootingWeaponBase* NewWeapon, bool bEquipWeapon)
{
	if (GetLocalRole() < ROLE_Authority) { return false; }
	// 存在只加子弹
	if(DoesWeaponExist(NewWeapon))
	{
		NewWeapon->Destroy();	
	}
	else
	{
		// 添加数据
		Inventory.Weapons.Add(NewWeapon);
		// 武器Actor 归this
		NewWeapon->SetOwningCharacter(this);
		NewWeapon->AddAbilitiesToASC(AbilitySystemComponent);
		// 模型位置 标记 CurrentWeapon
		EquipWeapon(NewWeapon);
	}

	return true;
}

bool AShootingCharacterBase::IsFirstPerson() const
{
	return Controller && Controller->IsLocalPlayerController();
}

void AShootingCharacterBase::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent,
		                                                              FGameplayAbilityInputBinds(
			                                                              FString("ConfirmTarget"),
			                                                              FString("CancelTarget"),
			                                                              FTopLevelAssetPath(TEXT("/Script/TX"),TEXT("EShootingInputID")),
			                                                              static_cast<int32>(
				                                                              EShootingInputID::CONFIRM),
			                                                              static_cast<int32>(
				                                                              EShootingInputID::CANCEL)));

		bASCInputBound = true;
	}	
}


