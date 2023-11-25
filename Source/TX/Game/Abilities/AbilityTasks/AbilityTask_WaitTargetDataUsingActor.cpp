// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WaitTargetDataUsingActor.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TX/Game/Abilities/Trace/ShootingTrace.h"

UAbilityTask_WaitTargetDataUsingActor* UAbilityTask_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
	UGameplayAbility* OwningAbility, FName TaskInstanceName,
	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor,
	bool bCreateKeyIfNotValidForMorePredicting)
{
	UAbilityTask_WaitTargetDataUsingActor* MyObj = NewAbilityTask<UAbilityTask_WaitTargetDataUsingActor>(OwningAbility,TaskInstanceName);
	MyObj->TargetActor = InTargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	MyObj->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;
	return MyObj;
}

void UAbilityTask_WaitTargetDataUsingActor::Activate()
{
	if(IsValid(this))
	{
		return;
	}

	//
	if(Ability && TargetActor)
	{
		InitializeTargetActor();
		RegisterTargetDataCallbacks();
		FinalizeTargetActor();
	}
	else
	{
		EndTask();
	}
}

void UAbilityTask_WaitTargetDataUsingActor::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	check(AbilitySystemComponent.Get());
	if (!Ability)
	{
		return;
	}

	// 预测
	// FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(),
	                                         // ShouldReplicateDataToServer() &&
	                                         // (bCreateKeyIfNotValidForMorePredicting &&!AbilitySystemComponent->ScopedPredictionKey.
		                                                                  // IsValidForMorePrediction()));

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	// if (IsPredictingClient())
	// {
		// if (!TargetActor->ShouldProduceTargetDataOnServer)
		// {
			// FGameplayTag ApplicationTag; // Fixme: where would this be useful?
			// AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, AbilitySystemComponent->ScopedPredictionKey);
		// }
		// else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		// {
			// We aren't going to send the target data, but we will send a generic confirmed message.
			// AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		// }
	// }

	// 告知委托过来的对象，技能数据已经准备好了
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		DataValid.Broadcast(Data);
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

// 技能被取消后执行
void UAbilityTask_WaitTargetDataUsingActor::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	check(AbilitySystemComponent.Get());

	// FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());

	// if (IsPredictingClient())
	// {
		// if (!TargetActor->ShouldProduceTargetDataOnServer)
		// {
			// AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		// }
		// else
		// {
			// We aren't going to send the target data, but we will send a generic confirmed message.
			// AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		// }
	// }

	// 告诉委托给自己的对象，技能被取消了
	Cancelled.Broadcast(Data);
	EndTask();
}

void UAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
	FGameplayTag ActivationTag)
{
}

void UAbilityTask_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback()
{
}

void UAbilityTask_WaitTargetDataUsingActor::OnDestroy(bool bInOwnerFinished)
{
	if(TargetActor)
	{
		AShootingTrace* TraceTargetActor = Cast<AShootingTrace>(TargetActor);
		if(TraceTargetActor)
		{
			// TraceTargetActor
		}
	}
}

bool UAbilityTask_WaitTargetDataUsingActor::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	// Send TargetData to the server IFF we are the client and this isn't a GameplayTargetActor that can produce data on the server
	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}

void UAbilityTask_WaitTargetDataUsingActor::InitializeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	// If we spawned the target actor, always register the callbacks for when the data is ready.
	// TargetActor->TargetDataReadyDelegate.AddUObject(this, &UAbilityTask_WaitTargetDataUsingActor::OnTargetDataReadyCallback);
	// TargetActor->CanceledDelegate.AddUObject(this, &UAbilityTask_WaitTargetDataUsingActor::OnTargetDataCancelledCallback);
}

void UAbilityTask_WaitTargetDataUsingActor::FinalizeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm)
			TargetActor->BindToConfirmCancelInputs();
		}
	}
}

void UAbilityTask_WaitTargetDataUsingActor::RegisterTargetDataCallbacks()
{
	if (!ensure(IsPendingKill() == false))
	{
		return;
	}

	check(Ability);

	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;

	// If not locally controlled (server for remote client), see if TargetData was already sent
	// else register callback for when it does get here.
	if (!bIsLocallyControlled)
	{
		// Register with the TargetData callbacks if we are expecting client to send them
		if (!bShouldProduceTargetDataOnServer)
		{
			FGameplayAbilitySpecHandle	SpecHandle = GetAbilitySpecHandle();
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

			// 
			// AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback);
			// AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback);

			AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			SetWaitingOnRemotePlayerData();
		}
	}
}
