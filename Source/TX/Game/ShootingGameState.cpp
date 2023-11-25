// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameState.h"

#include "Net/UnrealNetwork.h"


void AShootingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingGameState, NumTeams);
	DOREPLIFETIME(AShootingGameState, TeamScores);
	DOREPLIFETIME(AShootingGameState, bTimerPaused);
	DOREPLIFETIME(AShootingGameState, RemainingTime);
}

void AShootingGameState::AddTeamScore(int32 TeamNumber, float Score)
{
	if(TeamNumber >= 0)
	{
		if(TeamNumber >= TeamScores.Num())
		{
			TeamScores.AddZeroed(TeamNumber - TeamScores.Num() + 1);
		}
	}

	TeamScores[TeamNumber] += Score;
}
