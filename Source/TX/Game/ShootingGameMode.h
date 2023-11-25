// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShootingGameMode.generated.h"

class AShootingPlayerState;
/**
 * 
 */
UCLASS()
class TX_API AShootingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShootingGameMode();
	
	void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	int32 ChooseTeam(AShootingPlayerState* ForPlayerState) const;

protected:
	int32 NumTeams;
};
