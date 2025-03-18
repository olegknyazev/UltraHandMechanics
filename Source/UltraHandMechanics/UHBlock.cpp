#include "UHBlock.h"

#include "UHAttachable.h"
#include "UHBlockMovementComponent.h"
#include "GameFramework/MovementComponent.h"

UUHBlock::UUHBlock()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
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
		
		UpdateMaterial();
		
		if (!bManipulated && MovementComponent)
		{
			MovementComponent->StopMovementImmediately();
		}

		PrimitiveComponent->SetEnableGravity(!bManipulated);
		PrimitiveComponent->SetPhysMaterialOverride(bManipulated ? ManipulatedPhysicalMaterial : nullptr);
	}
}

UPrimitiveComponent* UUHBlock::GetPrimitiveComponent() const
{
	return PrimitiveComponent;
}

void UUHBlock::SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent)
{
	if (PrimitiveComponent != InPrimitiveComponent)
	{
		if (PrimitiveComponent && bSelectedMaterialApplied)
		{
			PrimitiveComponent->SetMaterial(0, OriginalMaterial);
		}
		
		PrimitiveComponent = InPrimitiveComponent;

		if (PrimitiveComponent && bSelectedMaterialApplied)
		{
			OriginalMaterial = PrimitiveComponent->GetMaterial(0);
			
			if (HighlightedMaterial)
			{
				PrimitiveComponent->SetMaterial(0, HighlightedMaterial);
			}
		}
	}
}

FVector UUHBlock::GetBlockLocation() const
{
	return PrimitiveComponent->GetComponentLocation();
}

FRotator UUHBlock::GetBlockRotation() const
{
	return PrimitiveComponent->GetComponentRotation();
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
}

void UUHBlock::UpdateMaterial()
{
	if (PrimitiveComponent)
	{
		if (!bSelectedMaterialApplied)
		{
			OriginalMaterial = PrimitiveComponent->GetMaterial(0);
		}

		bSelectedMaterialApplied = bHighlighted || bManipulated;

		PrimitiveComponent->SetMaterial(0, (bSelectedMaterialApplied && HighlightedMaterial) ? HighlightedMaterial : OriginalMaterial);
	}
}

