// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShootingGameState.generated.h"

/**
 * 
 */
UCLASS()
class TX_API AShootingGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddTeamScore(int32 TeamNumber, float Score);

	UPROPERTY(Transient, Replicated)
	int32 NumTeams;

	UPROPERTY(Transient, Replicated)
	TArray<int32> TeamScores;
	
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	UPROPERTY(Transient, Replicated)
	bool bTimerPaused;

};
