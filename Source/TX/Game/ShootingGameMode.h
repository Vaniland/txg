// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShootingGameMode.generated.h"

class AShootingCharacterBase;
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

	virtual void PreInitializeComponents() override;

	void Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	int32 ChooseTeam(AShootingPlayerState* ForPlayerState) const;

	void RestartPlayerCharacter(AController* NewPlayer);

	void DefaultTimer();

protected:
	UPROPERTY(EditDefaultsOnly)
	int32 NumTeams;

	UPROPERTY(EditDefaultsOnly)
	int32 RoundTime;

	FTimerHandle TimerHandle_DefaultTimer;
};
