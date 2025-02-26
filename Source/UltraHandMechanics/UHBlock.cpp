#include "UHBlock.h"

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

void UUHBlock::BeginPlay()
{
	Super::BeginPlay();
}

void UUHBlock::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

