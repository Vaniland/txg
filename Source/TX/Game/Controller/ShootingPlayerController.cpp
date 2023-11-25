// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerController.h"

bool AShootingPlayerController::IsGameInputAllowed() const
{
	return true;
	// return bAllowGameActions && !bCinematicMode;
}

void AShootingPlayerController::UnFreeze()
{
	ServerRestartPlayer();
}
