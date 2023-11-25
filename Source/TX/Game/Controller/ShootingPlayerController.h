// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShootingPlayerController.generated.h"

DECLARE_DELEGATE_OneParam(FOnWeaponEquippedDelegate, UTexture2D*);
DECLARE_DELEGATE_OneParam(FOnWeaponAmmoChangedDelegate, const FText&);
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
	
	// 是否允许输入
	bool IsGameInputAllowed() const;

	virtual void UnFreeze() override;


protected:
	bool bAllowGameActions;
};
