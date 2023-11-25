// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_ShootingMain.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
/**
 * 
 */
UCLASS()
class TX_API UUI_ShootingMain : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void SetHP(float HealthPercent);
	void SetWeaponIcon(UTexture2D* WeaponIconTexture);
	void SetAmmoText(int32 CurrentAmmoInClip, int32 RemainAmmo);
	void SetAmmoText(const FText& AmmoText);

	void SetKillsText(int32 KillsCount);

private:
	UPROPERTY(meta = (BindWidget))
	class UUI_CrossHair* CrossHair;

private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillsTextBlock;
	
};
