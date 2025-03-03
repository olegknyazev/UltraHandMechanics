#include "UHBlock.h"

#include "UHBlockMovementComponent.h"
#include "GameFramework/MovementComponent.h"

UUHBlock::UUHBlock()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UUHBlock::IsHighlighted() const
{
	return bHighlighted;
}

void UUHBlock::SetHighlighted(bool bInHighlighted)
{
	if (bHighlighted != bInHighlighted)
	{
		if (HighlightablePrimitive)
		{
			if (!bHighlighted)
			{
				OriginalMaterial = HighlightablePrimitive->GetMaterial(0);
			}

			HighlightablePrimitive->SetMaterial(0, (bInHighlighted && HighlightedMaterial) ? HighlightedMaterial : OriginalMaterial);
		}
		
		bHighlighted = bInHighlighted;
	}
}

UPrimitiveComponent* UUHBlock::GetPrimitiveComponent() const
{
	return GetOwner()->FindComponentByClass<UPrimitiveComponent>();
}

FVector UUHBlock::GetBlockLocation() const
{
	return GetPrimitiveComponent()->GetComponentLocation();
}

void UUHBlock::SetTargetPlacement(const FVector& Location, const FRotator& Rotation)
{
	TargetLocation = Location;
	TargetRotation = Rotation;
}

void UUHBlock::ResetTargetPlacement()
{
	TargetLocation.Reset();
	TargetRotation.Reset();

	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
	}
}

void UUHBlock::BeginPlay()
{
	Super::BeginPlay();
}

void UUHBlock::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TargetLocation.IsSet())
	{
		if (MovementComponent)
		{
			MovementComponent->Velocity = (*TargetLocation - GetOwner()->GetActorLocation()) / TargetApproachTime;
			MovementComponent->AngularVelocity = (*TargetRotation - GetOwner()->GetActorRotation()).GetNormalized() * (1.f / TargetApproachTime);
		}
	}
}

