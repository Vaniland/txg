// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacterAnimInstanceBase.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TX/Game/Character/ShootingCharacterBase.h"

UShootingCharacterAnimInstanceBase::UShootingCharacterAnimInstanceBase()
	: Speed(0.0f)
	  , bDead(false)
	  , bInAir(false)
{
}

void UShootingCharacterAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UShootingCharacterAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(AShootingCharacterBase* InCharacter = Cast<AShootingCharacterBase>(TryGetPawnOwner()))
	{
		Speed = InCharacter->GetVelocity().Size();
		Direction =  UKismetAnimationLibrary::CalculateDirection(InCharacter->GetVelocity(), InCharacter->GetActorRotation());
		bCrouch = InCharacter->GetCharacterMovement()->IsCrouching();
		bInAir = InCharacter->GetCharacterMovement()->IsFalling();
	}
	
}
