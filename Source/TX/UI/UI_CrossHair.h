// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI_CrossHair.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class TX_API UUI_CrossHair : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetShowCrossHair(bool bShow);

	UPROPERTY(EditDefaultsOnly)
	float SpreadMulti = 10.f;

private:

	// CrossHair
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairC;
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairU;
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairD;
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairL;
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairR;
};


