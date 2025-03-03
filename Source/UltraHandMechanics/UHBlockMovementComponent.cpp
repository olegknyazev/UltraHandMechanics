#include "UHBlockMovementComponent.h"


UUHBlockMovementComponent::UUHBlockMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHBlockMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUHBlockMovementComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FRotator RotationDelta = AngularVelocity * DeltaTime;
	const FVector MoveDelta = Velocity * DeltaTime;

	if (!MoveDelta.IsNearlyZero() || !RotationDelta.IsNearlyZero())
	{
		const FRotator Rotation = UpdatedComponent->GetComponentRotation() + RotationDelta;
		
		FHitResult Hit;
		SafeMoveUpdatedComponent(MoveDelta, Rotation, true, Hit);
		if (Hit.bBlockingHit)
		{
			SlideAlongSurface(MoveDelta, 1 - Hit.Time, Hit.Normal, Hit, false);
		}
	}
}

void UUHBlockMovementComponent::StopMovementImmediately()
{
	Super::StopMovementImmediately();
	
	AngularVelocity = FRotator::ZeroRotator;
}

