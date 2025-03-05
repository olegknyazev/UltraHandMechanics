#include "UHBlock.h"

#include "UHAttachable.h"
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
		bHighlighted = bInHighlighted;
		UpdateMaterial();
	}
}

bool UUHBlock::IsManipulated() const
{
	return bManipulated;
}

void UUHBlock::SetManipulated(bool bInManipulated)
{
	if (bManipulated != bInManipulated)
	{
		bManipulated = bInManipulated;
		TargetLocation = GetBlockLocation();
		TargetRotation = GetBlockRotation();
		
		UpdateMaterial();
		
		if (!bManipulated && MovementComponent)
		{
			MovementComponent->StopMovementImmediately();
		}

		GetPrimitiveComponent()->SetSimulatePhysics(!bManipulated);
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

FRotator UUHBlock::GetBlockRotation() const
{
	return GetPrimitiveComponent()->GetComponentRotation();
}

void UUHBlock::SetTargetPlacement(const FVector& Location, const FRotator& Rotation)
{
	TargetLocation = Location;
	TargetRotation = Rotation;
}

bool UUHBlock::StartSticking()
{
	if (Attachable)
	{
		if (Attachable->StartSticking())
		{
			if (bManipulated)
			{
				bManipulated = false;
				if (MovementComponent)
				{
					MovementComponent->StopMovementImmediately();
				}
				UpdateMaterial();
			}
			
			return true;
		}
	}

	return false;
}

void UUHBlock::BeginPlay()
{
	Super::BeginPlay();
}

void UUHBlock::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bManipulated)
	{
		if (MovementComponent)
		{
			MovementComponent->Velocity = (TargetLocation - GetOwner()->GetActorLocation()) / TargetApproachTime;
			MovementComponent->AngularVelocity = (TargetRotation - GetOwner()->GetActorRotation()).GetNormalized() * (1.f / TargetApproachTime);
		}
	}
}

void UUHBlock::UpdateMaterial()
{
	if (HighlightablePrimitive)
	{
		if (!bSelectedMaterialApplied)
		{
			OriginalMaterial = HighlightablePrimitive->GetMaterial(0);
		}

		bSelectedMaterialApplied = bHighlighted || bManipulated;

		HighlightablePrimitive->SetMaterial(0, (bSelectedMaterialApplied && HighlightedMaterial) ? HighlightedMaterial : OriginalMaterial);
	}
}

