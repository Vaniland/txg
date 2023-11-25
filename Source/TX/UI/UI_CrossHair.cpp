// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_CrossHair.h"

#include "TX/Game/Character/ShootingCharacterBase.h"
#include "TX/Game/Controller/ShootingPlayerController.h"
#include "TX/Game/Weapons/ShootingWeapon_Instant.h"

void UUI_CrossHair::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AShootingPlayerController* PC = Cast<AShootingPlayerController>(GetOwningPlayer());
	if (!PC) { return; }
	
	AShootingCharacterBase* PlayerPawn = Cast<AShootingCharacterBase>(PC->GetPawn());
	if (PlayerPawn && PlayerPawn->GetCurrentWeapon())
	{
		AShootingWeapon_Instant* InstantWeapon = Cast<AShootingWeapon_Instant>(PlayerPawn->GetCurrentWeapon());

		float CrossHairSpread = 0.0f;
		if (InstantWeapon)
		{
			CrossHairSpread += SpreadMulti * InstantWeapon->GetCurrentSpread();

			// GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red,
			                                 // FString::Printf(TEXT("CrossHairSpread : %f"), CrossHairSpread));
			
			if(CrossHairU)
			{
				CrossHairU->SetRenderTranslation(FVector2D(0.f, CrossHairSpread));
			}
			if(CrossHairD)
			{
				CrossHairD->SetRenderTranslation(FVector2D(0.f, -CrossHairSpread));
			}
			if(CrossHairL)
			{
				CrossHairL->SetRenderTranslation(FVector2D(-CrossHairSpread, 0.f));
			}
			if(CrossHairR)
			{
				CrossHairR->SetRenderTranslation(FVector2D(CrossHairSpread, 0.f));
			}
		}
	}
}

void UUI_CrossHair::SetShowCrossHair(bool bShow)
{
	if(bShow)
	{
		CrossHairU->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CrossHairD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CrossHairL->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CrossHairR->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		CrossHairU->SetVisibility(ESlateVisibility::Hidden);
		CrossHairD->SetVisibility(ESlateVisibility::Hidden);
		CrossHairL->SetVisibility(ESlateVisibility::Hidden);
		CrossHairR->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUI_CrossHair::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUI_CrossHair::NativeDestruct()
{
	Super::NativeDestruct();
}
