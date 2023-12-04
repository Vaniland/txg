// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShootingPlayerController.generated.h"

enum EWeaponType : uint8;
class AShootingCharacterBase;
DECLARE_DELEGATE_OneParam(FOnWeaponEquippedDelegate, UTexture2D*);
DECLARE_DELEGATE_OneParam(FOnWeaponAmmoChangedDelegate, const FText&);
DECLARE_DELEGATE_TwoParams(FOnWeaponPickDropDelegate, EWeaponType, bool);
DECLARE_DELEGATE_OneParam(FOnHealthChangedDelegate, float);
/**
 * 
 */
UCLASS()
class TX_API AShootingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// UI
	FOnHealthChangedDelegate OnHealthChangedDelegate;
	FOnWeaponEquippedDelegate OnWeaponEquippedDelegate;
	FOnWeaponAmmoChangedDelegate OnWeaponAmmoChangedDelegate;
	FOnWeaponPickDropDelegate OnWeaponPickDropDelegate;

	// 是否允许输入
	bool IsGameInputAllowed() const;

	void SpawnPlayerCharacter(const FTransform& SpawnTransform);

	virtual void UnFreeze() override;

	UFUNCTION(Reliable, Client)
	void OnEndGame();


protected:
	bool bAllowGameActions;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AShootingCharacterBase> PlayerCharacterClass;
};
