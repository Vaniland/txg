// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_ShootingMain.h"

#include "UI_CrossHair.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TX/Game/ShootingPlayerState.h"
#include "TX/Game/Character/ShootingCharacterBase.h"
#include "TX/Game/Controller/ShootingPlayerController.h"

void UUI_ShootingMain::NativeConstruct()
{
	Super::NativeConstruct();

	if(AShootingPlayerController* ShootingPlayerController = GetOwningPlayer<AShootingPlayerController>())
	{
		ShootingPlayerController->OnWeaponEquippedDelegate.BindUObject(this, &UUI_ShootingMain::SetWeaponIcon);
		ShootingPlayerController->OnHealthChangedDelegate.BindUObject(this, &UUI_ShootingMain::SetHP);
		ShootingPlayerController->OnWeaponAmmoChangedDelegate.BindUObject(this, &UUI_ShootingMain::SetAmmoText);
		ShootingPlayerController->OnWeaponPickDropDelegate.BindUObject(this, &UUI_ShootingMain::UpdateWeaponInventoryIcon);
	}
	
}

void UUI_ShootingMain::NativeDestruct()
{
	Super::NativeDestruct();
}

void UUI_ShootingMain::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if(AShootingCharacterBase* PlayerCharacter = GetOwningPlayerPawn<AShootingCharacterBase>())
	{
		SetHP(PlayerCharacter->GetHealthPercent());
		SetAmmoText(PlayerCharacter->GetAmmoText());
	}
	else
	{
		SetHP(0.f);
		SetAmmoText(FText::FromString(TEXT("NULL")));
		SetWeaponIcon(nullptr);
	}

	if(AShootingPlayerState* ShootingPlayerState = GetOwningPlayerState<AShootingPlayerState>())
	{
		SetKillsText(ShootingPlayerState->GetKills());
	}

}

void UUI_ShootingMain::SetHP(float HealthPercent)
{
	HPBar->SetPercent(HealthPercent);
}

void UUI_ShootingMain::SetWeaponIcon(UTexture2D* WeaponIconTexture)
{
	WeaponIcon->SetBrushFromTexture(WeaponIconTexture);
}

void UUI_ShootingMain::SetAmmoText(int32 CurrentAmmoInClip, int32 RemainAmmo)
{
	// AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmoInClip, RemainAmmo)));
	SetAmmoText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmoInClip, RemainAmmo)));
}

void UUI_ShootingMain::SetAmmoText(const FText& AmmoText)
{
	AmmoTextBlock->SetText(AmmoText);
}

void UUI_ShootingMain::UpdateWeaponInventoryIcon(EWeaponType WeaponType, bool bPickDrop)
{
}


void UUI_ShootingMain::SetKillsText(int32 KillsCount)
{
	KillsTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Kills: %d"), KillsCount)));
}

void UUI_ShootingMain::NotifyEnemyHit()
{
	if(CrossHair)
	{
		CrossHair->EnemyHitBlink();
	}
}

void UUI_ShootingMain::NotifyEnemyKilled(bool bHeadShot)
{
	CrossHair->EnemyHitBlink();
	K2_NotifyEnemyKilled(bHeadShot);
}
