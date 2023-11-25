// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectApplied_Target.h"
#include "AbilityTask_WaitTargetDataUsingActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTargetDataUsingActorDelegate, const FGameplayAbilityTargetDataHandle&,Data);
class AGameplayAbilityTargetActor;

/**
 * 
 */
UCLASS()
class TX_API UAbilityTask_WaitTargetDataUsingActor : public UAbilityTask_WaitGameplayEffectApplied_Target
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate DataValid;

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate Cancelled;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAbilityTask_WaitTargetDataUsingActor* WaitTargetDataWithReusableActor(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor,
		bool bCreateKeyIfNotValidForMorePredicting = false
	);

	virtual void Activate() override;

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();

	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	virtual bool ShouldReplicateDataToServer() const;

	virtual void InitializeTargetActor() const;
	virtual void FinalizeTargetActor() const;

	void RegisterTargetDataCallbacks();
protected:
	UPROPERTY()
	AGameplayAbilityTargetActor* TargetActor;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	bool bCreateKeyIfNotValidForMorePredicting;

};


