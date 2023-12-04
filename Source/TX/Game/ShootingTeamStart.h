// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "ShootingTeamStart.generated.h"

UCLASS()
class TX_API AShootingTeamStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShootingTeamStart(const FObjectInitializer& ObjectInitializer);

	int32 GetTeamNumber() const { return TeamNumber; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditInstanceOnly, Category="Team")
	int32 TeamNumber;
};
