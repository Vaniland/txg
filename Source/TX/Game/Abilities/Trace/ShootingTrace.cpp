// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingTrace.h"

#include "Abilities/GameplayAbility.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AShootingTrace::AShootingTrace()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AShootingTrace::StartTargeting(UGameplayAbility* Ability)
{
	SetActorTickEnabled(true);

	OwningAbility = Ability;
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

}

FGameplayAbilityTargetDataHandle AShootingTrace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		/** Note: These are cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
		FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
		ReturnData->HitResult = HitResults[i];
		ReturnDataHandle.Add(ReturnData);
	}

	return ReturnDataHandle;
}

void AShootingTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData())
	if(SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
		}
#endif
	}
}

void AShootingTrace::CancelTargeting()
{
	Super::CancelTargeting();
}

void AShootingTrace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
                                         const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                                         const FVector& End, FName ProfileName,
                                         const FCollisionQueryParams Params)
{
	check(World)

	TArray<FHitResult> HitResults;
	// 用ProfileName来检测
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	TArray<FHitResult> FilteredHitResults;

	// 开始位置可能是玩家视点，我们希望HitResult始终显示StartLocation而不是viewpoint
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.GetActor() || FilterHandle.FilterPassesForActor(Hit.GetActor()))
		{
			Hit.TraceStart = TraceStart;
			Hit.TraceEnd = End;

			FilteredHitResults.Add(Hit);
		}
	}

	OutHitResults = FilteredHitResults;

}

void AShootingTrace::StopTargeting()
{
	SetActorTickEnabled(false);

	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();
}

// Called when the game starts or when spawned
void AShootingTrace::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
	
}

TArray<FHitResult> AShootingTrace::PerformTrace(AActor* InSourceActor)
{
	// 忽略Actor集合
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	// 碰撞检测
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShootingLineTrace),false);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	// Params.bIgnoreBlocks = 
	
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	FRotator ViewRot = InSourceActor->GetTransform().GetRotation().Rotator();
	FVector TraceDir;

	if(PrimaryPC)
	{
		FVector ViewStart;
		// FRotator ViewRot;
		PrimaryPC->GetPlayerViewPoint(ViewStart,ViewRot);

		// 如果是玩家视角,则从玩家视角开始发射
		TraceStart = bTraceFromPlayerViewPoint ? ViewStart : StartLocation.GetTargetingTransform().GetLocation();
	}
	TraceEnd = TraceStart + TraceStart + ViewRot.Vector() * MaxRange;

	TArray<FHitResult> ReturnHitResults;
	// LineTraceWithFilter(ReturnHitResults);
	for(int32 TraceIndex = 0; TraceIndex < NumberOfTraces; ++TraceIndex)
	{
		TArray<FHitResult> TraceHitResult;
		LineTraceWithFilter(TraceHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

		for (int32 HitIndex = TraceHitResult.Num() - 1; HitIndex >= 0; --HitIndex)
		{
			// 剔除超出最大穿透次数的hit
			if (MaxHitResultsPerTrace >= 0 && HitIndex + 1 > MaxHitResultsPerTrace)
			{
				TraceHitResult.RemoveAt(HitIndex);
				continue;
			}
		}
	}

	// 找到扩散之后的EndPoint
	// do
	// {
	// 	if (!OwningAbility) { break; }
	// 	FVector ViewStart = TraceStart;
	// 	// 面向方向
	// 	FRotator ViewRot = OwningAbility->GetCurrentActorInfo()->AvatarActor->GetActorRotation();
	//
	// 	// 如果是玩家视角,则从视点开始?
	// 	if (PrimaryPC) { PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot); }
	// 	const FVector ViewDir = ViewRot.Vector();
	// 	FVector ViewEnd = ViewStart + ViewDir * MaxRange;
	//
	// 	// 用技能范围来限制检测范围?
	// 	// TODO
	//
	// 	TArray<FHitResult> HitResults;
	// 	// TODO 扩散 采用第一个hit位置加上扩散改变endpoint (还要结合武器射程)这里主要确定dir
	//
	//
	// 	// TODO 改变玩家视角抖动 ptich
	// }
	// while (false);	


	return ReturnHitResults;
}

void AShootingTrace::ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType, float Duration)
{
#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		FVector ViewStart = StartLocation.GetTargetingTransform().GetLocation();
		FRotator ViewRot;
		if (PrimaryPC && bTraceFromPlayerViewPoint)
		{
			PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot);
		}

		FVector TraceEnd = HitResults[0].TraceEnd;
		if (NumberOfTraces > 1 || bUsePersistentHitResults)
		{
			TraceEnd = CurrentTraceEnd;
		}

		// DrawDebugSphereTraceMulti(GetWorld(), ViewStart, TraceEnd, TraceSphereRadius, DrawDebugType, true, HitResults, FLinearColor::Green, FLinearColor::Red, Duration);
	}
#endif
}

// Called every frame
void AShootingTrace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

