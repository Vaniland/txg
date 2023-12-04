// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "TX/Game/Character/ShootingCharacterBase.h"

bool AShootingPlayerController::IsGameInputAllowed() const
{
	return true;
	// return bAllowGameActions && !bCinematicMode;
}

void AShootingPlayerController::SpawnPlayerCharacter(const FTransform& SpawnTransform)
{
	if (!PlayerCharacterClass) { return; }
	
	FTimerHandle TimerHandle_SpawnActor;
	SetControlRotation(SpawnTransform.GetRotation().Rotator());
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnActor, [this, SpawnTransform]()
	{
		this->SetControlRotation(SpawnTransform.GetRotation().Rotator());
		if(AShootingCharacterBase* ShootingCharacter = GetWorld()->SpawnActor<AShootingCharacterBase>(PlayerCharacterClass, SpawnTransform))
		{
			Possess(ShootingCharacter);
		}
	}, 1.f, false);
}

void AShootingPlayerController::UnFreeze()
{
	ServerRestartPlayer();
}

void AShootingPlayerController::OnEndGame_Implementation()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Maps/MainMenu"));
}
