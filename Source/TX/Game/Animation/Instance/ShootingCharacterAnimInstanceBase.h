// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShootingCharacterAnimInstanceBase.generated.h"

/**
 * 
 */
UCLASS()
class TX_API UShootingCharacterAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
public:
	UShootingCharacterAnimInstanceBase();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AnimAttribute")
	float Speed;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AnimAttribute")
	float Direction;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AnimAttribute")
	bool bDead;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AnimAttribute")
	bool bInAir;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AnimAttribute")
	bool bCrouch;
	
};
