// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EShootingInputID : uint8
{
	NONE = 0					UMETA(DisplayName = "None"),
	CONFIRM = 1					UMETA(DisplayName = "Confirm"),
	CANCEL = 2					UMETA(DisplayName = "Cancel"),
	PRIMARYFIRE = 3				UMETA(DisplayName = "PrimaryFire"),
};