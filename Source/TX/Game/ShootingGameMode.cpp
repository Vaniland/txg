// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingGameMode.h"

#include "EngineUtils.h"
#include "ShootingGameState.h"
#include "ShootingPlayerState.h"
#include "ShootingTeamStart.h"
#include "Controller/ShootingPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"

AShootingGameMode::AShootingGameMode()
	:NumTeams(2)
	,RoundTime(90)
{
}

void AShootingGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if(AShootingGameState* GS = Cast<AShootingGameState>(GameState))
	{
		GS->RemainingTime = RoundTime;
	}
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AShootingGameMode::DefaultTimer, 1.f,true);
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

	RestartPlayerCharacter(NewPlayer);
	
	Super::PostLogin(NewPlayer);
}



AActor* AShootingGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;

	// 搜索所有start PIE优先， 然后是TeamStart
	APlayerStart* BestStart = nullptr;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		// play from here?
		if(TestSpawn->IsA<APlayerStartPIE>())
		{
			BestStart = TestSpawn;
			break;
		}

		AShootingPlayerState* PlayerState = Player ? Cast<AShootingPlayerState>(Player->PlayerState) : nullptr;
		AShootingTeamStart* TeamStart = TestSpawn ? Cast<AShootingTeamStart>(TestSpawn) : nullptr;
		if (TeamStart && PlayerState && PlayerState->GetTeamNumber() == TeamStart->GetTeamNumber())
		{
			PreferredSpawns.Add(TestSpawn);
		}
	}

	if(BestStart == nullptr)
	{
		if(PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

int32 AShootingGameMode::ChooseTeam(AShootingPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
	TeamBalance.AddZeroed(NumTeams);


	// 找到当前队伍人数
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

void AShootingGameMode::RestartPlayerCharacter(AController* NewPlayer)
{
	AShootingPlayerController* NewPlayerController = CastChecked<AShootingPlayerController>(NewPlayer);
	AActor* const StartSpot = ChoosePlayerStart(NewPlayer);
	
	if (StartSpot)
	{
		NewPlayerController->SpawnPlayerCharacter(StartSpot->GetTransform());
	}
}

void AShootingGameMode::DefaultTimer()
{
	AShootingGameState* const MyGameState = Cast<AShootingGameState>(GameState);
	if(MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;
		if(MyGameState->RemainingTime <= 0)
		{
			// 结束游戏
			// 遍历所有玩家
			for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				AShootingPlayerController* PC = Cast<AShootingPlayerController>(It->Get());
				if(PC && PC->IsLocalController())
				{
					PC->OnEndGame();
				}
			}
			
			Cast<AShootingGameState>(GameState)->RemainingTime = RoundTime;
		}
	}
}
