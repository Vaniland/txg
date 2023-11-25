// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShootingHUD.generated.h"

class UUI_ShootingMain;
/**
 * 
 */
UCLASS()
class TX_API AShootingHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUI_ShootingMain> MainUIClass;
	
	AShootingHUD();

	virtual void BeginPlay() override;

private:
	UUI_ShootingMain* MainUI;
};
