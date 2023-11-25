// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShootingPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TX_API AShootingPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void SetTeamNumber(int32 NewTeamNumber);
	
	int32 GetTeamNumber() const;

	void ScoreKill(AShootingPlayerState* Victim, int32 Points);

	int32 GetKills() const;

	void ScoreDeath(AShootingPlayerState* KilledBy, int32 Points);

	int32 GetDeaths() const;

	void ScorePoints(int32 Points);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamNumber)
	int32 TeamNumber;
	
	UPROPERTY(Transient, Replicated)
	int32 NumKills;
	
	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;

	UFUNCTION()
	void OnRep_TeamNumber();

};
