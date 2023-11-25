// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameMode.h"

#include "ShootingPlayerState.h"
#include "GameFramework/GameStateBase.h"

AShootingGameMode::AShootingGameMode()
	:NumTeams(2)
{
}

void AShootingGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn,
                               const UDamageType* DamageType)
{
	AShootingPlayerState* KillerPlayerState = Killer ? Cast<AShootingPlayerState>(Killer->PlayerState) : nullptr;
	AShootingPlayerState* VictimPlayerState = VictimPlayer ? Cast<AShootingPlayerState>(VictimPlayer->PlayerState) : nullptr;

	// 自杀不计分
	if(KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, 1);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, 0);	
	}
}

void AShootingGameMode::PostLogin(APlayerController* NewPlayer)
{
	// 进来先选边 加入人最少的
	AShootingPlayerState* NewPlayerState = CastChecked<AShootingPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNumber(TeamNum);
	UE_LOG(LogTemp,Warning,TEXT("%s TeamNum:%d"), *NewPlayerState->GetName() ,TeamNum);
	
	Super::PostLogin(NewPlayer);
}

int32 AShootingGameMode::ChooseTeam(AShootingPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(NumTeams);

	// 找到队伍数量
	for(int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
	{
		AShootingPlayerState const * const TestPlayerState = Cast<AShootingPlayerState>(GameState->PlayerArray[i]);
		if(TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNumber()))
		{
			TeamBalance[TestPlayerState->GetTeamNumber()]++;
		}
	}

	// 找到人最少的队伍人数
	int32 LeastPopularNum = TeamBalance[0];
	for(int32 i = 1; i < TeamBalance.Num(); ++i)
	{
		if(TeamBalance[i] < LeastPopularNum)
		{
			LeastPopularNum = TeamBalance[i];
		}
	}

	TArray<int32> TeamsToChoose;
	for(int32 i = 0; i < TeamBalance.Num(); ++i)
	{
		if(TeamBalance[i] == LeastPopularNum)
		{
			TeamsToChoose.Add(i);
		}
	}

	const int32 TeamToChoose = TeamsToChoose[FMath::RandHelper(TeamsToChoose.Num())];
	return TeamToChoose;
}
