// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacterMovementComponent.h"

#include "TX/Game/Character/ShootingCharacterBase.h"


// Sets default values for this component's properties
UShootingCharacterMovementComponent::UShootingCharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UShootingCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UShootingCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UShootingCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();
	const AShootingCharacterBase* ShootingCharacter = Cast<AShootingCharacterBase>(GetOwner());
	if(ShootingCharacter)
	{
		if(ShootingCharacter->IsSprinting())
		{
			MaxSpeed *= 1.5f;
		}
	}

	return MaxSpeed;
}

