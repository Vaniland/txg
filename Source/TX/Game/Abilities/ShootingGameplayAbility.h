// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ShootingGameplayAbility.generated.h"

enum class EShootingInputID : uint8;
/**
 * 
 */
UCLASS()
class TX_API UShootingGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UShootingGameplayAbility();

	EShootingInputID GetInputID() { return InputID; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "ShootingAbility")
	EShootingInputID InputID;
	
};
