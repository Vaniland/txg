// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerState.h"

#include "ShootingGameState.h"
#include "Net/UnrealNetwork.h"

void AShootingPlayerState::SetTeamNumber(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
}

int32 AShootingPlayerState::GetTeamNumber() const
{
	return TeamNumber;
}

void AShootingPlayerState::ScoreKill(AShootingPlayerState* Victim, int32 Points)
{
	++NumKills;
	ScorePoints(Points);
}

int32 AShootingPlayerState::GetKills() const
{
	return NumKills;
}

void AShootingPlayerState::ScoreDeath(AShootingPlayerState* KilledBy, int32 Points)
{
	++NumDeaths;
	ScorePoints(Points);
}

int32 AShootingPlayerState::GetDeaths() const
{
	return NumDeaths; 
}

void AShootingPlayerState::ScorePoints(int32 Points)
{
	AShootingGameState* const MyGameState = GetWorld()->GetGameState<AShootingGameState>();
	if(MyGameState && TeamNumber >= 0)
	{
		MyGameState->AddTeamScore(TeamNumber, Points);
	}

	// UE自带 自己的分数?
	SetScore(GetScore() + Points);
}

void AShootingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingPlayerState, TeamNumber);
	DOREPLIFETIME(AShootingPlayerState, NumKills);
	DOREPLIFETIME(AShootingPlayerState, NumDeaths);
}

void AShootingPlayerState::OnRep_TeamNumber()
{
}
