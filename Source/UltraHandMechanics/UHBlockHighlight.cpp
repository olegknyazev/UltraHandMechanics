#include "UHBlockHighlight.h"


UUHBlockHighlight::UUHBlockHighlight()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UUHBlockHighlight::IsUnderAim() const
{
	return bUnderAim;
}

void UUHBlockHighlight::SetUnderAim(bool bInUnderAim)
{
	if (bUnderAim != bInUnderAim)
	{
		bUnderAim = bInUnderAim;
		
		UpdateMaterial();
	}
}

bool UUHBlockHighlight::IsManipulated() const
{
	return bManipulated;
}

void UUHBlockHighlight::SetManipulated(bool bInManipulated)
{
	if (bManipulated != bInManipulated)
	{
		bManipulated = bInManipulated;
		
		UpdateMaterial();
	}
}

UPrimitiveComponent* UUHBlockHighlight::GetPrimitiveComponent() const
{
	return PrimitiveComponent;
}

void UUHBlockHighlight::SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent)
{
	if (PrimitiveComponent != InPrimitiveComponent)
	{
		if (PrimitiveComponent && bMaterialApplied)
		{
			PrimitiveComponent->SetMaterial(0, OriginalMaterial);
		}
		
		PrimitiveComponent = InPrimitiveComponent;

		if (PrimitiveComponent && bMaterialApplied)
		{
			OriginalMaterial = PrimitiveComponent->GetMaterial(0);
			
			if (HighlightedMaterial)
			{
				PrimitiveComponent->SetMaterial(0, HighlightedMaterial);
			}
		}
	}
}

void UUHBlockHighlight::UpdateMaterial()
{
	if (PrimitiveComponent)
	{
		if (!bMaterialApplied)
		{
			OriginalMaterial = PrimitiveComponent->GetMaterial(0);
		}

		bMaterialApplied = bUnderAim || bManipulated;

		PrimitiveComponent->SetMaterial(0, (bMaterialApplied && HighlightedMaterial) ? HighlightedMaterial : OriginalMaterial);
	}
}

