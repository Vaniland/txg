// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ShootingTrace.generated.h"

UCLASS()
class TX_API AShootingTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShootingTrace();

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;

	// Trace
	virtual void LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
	                                 const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
	                                 const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	virtual void StopTargeting();


public:
	bool bTraceFromPlayerViewPoint;

	float MaxRange;

	// 一次判定次数?
	int32 NumberOfTraces;

	// 一次判定最大命中数
	int32 MaxHitResultsPerTrace;

	// 用于debug
	FVector CurrentTraceEnd;

	// HitResult保存到确认目标或取消后再或者是一个新的HitResult取代
	bool bUsePersistentHitResults;

	FCollisionProfileName TraceProfile;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);

	FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitResults) const;

	void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType, float Duration);

	

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
