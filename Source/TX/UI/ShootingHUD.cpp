// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingHUD.h"

#include "UI_ShootingMain.h"
#include "Blueprint/UserWidget.h"

AShootingHUD::AShootingHUD()
{
}

void AShootingHUD::BeginPlay()
{
	Super::BeginPlay();
	if(MainUIClass)
	{
		MainUI = CreateWidget<UUI_ShootingMain>(GetWorld(), MainUIClass);
		if(MainUI)
		{
			MainUI->AddToViewport();
		}	
	}
}
