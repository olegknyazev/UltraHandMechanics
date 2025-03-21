#include "UHPicker.h"

#include "UHBaseBlock.h"


DEFINE_LOG_CATEGORY(LogUHPicker);


UUHPicker::UUHPicker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

AUHBaseBlock* UUHPicker::GetSelectedBlock() const
{
	return SelectedBlock;
}

UPrimitiveComponent* UUHPicker::GetSelectedBlockPart() const
{
	return SelectedBlockPart;
}

void UUHPicker::SetPickingEnabled(bool bInEnabled)
{
	bPickingEnabled = bInEnabled;

	if (!bPickingEnabled)
	{
		SetSelectedPart(nullptr);
	}
}

bool UUHPicker::IsPickingEnabled() const
{
	return bPickingEnabled;
}

void UUHPicker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPickingEnabled)
	{
		SetSelectedPart(TraceMeshUnderAim());
	}
}

void UUHPicker::SetSelectedPart(UPrimitiveComponent* Part)
{
	auto* const Block = Part ? Cast<AUHBaseBlock>(Part->GetOwner()) : nullptr;
	auto* const PartToSelect = Block ? Part : nullptr;
	
	bool bBlockChanged = false;
	if (SelectedBlock != Block)
	{
		if (SelectedBlock)
		{
			SelectedBlock->SetHighlightedPart(nullptr);
		}

		SelectedBlock = Block;
		bBlockChanged = true;
	}

	if (bBlockChanged || SelectedBlockPart != PartToSelect)
	{
		if (SelectedBlock)
		{
			SelectedBlock->SetHighlightedPart(PartToSelect);
		}
		
		SelectedBlockPart = PartToSelect;

		UE_LOG(LogUHPicker, Display, TEXT("Selected block is now %s (%s)"),
        	SelectedBlock ? *SelectedBlock->GetName() : TEXT("None"),
        	SelectedBlockPart ? *SelectedBlockPart->GetName() : TEXT("None"));
	}
}

UPrimitiveComponent* UUHPicker::TraceMeshUnderAim() const
{
	auto* const PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
	{
		return nullptr;
	}

	const APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	int32 ViewportWidth, ViewportHeight;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

	FHitResult Hit;
	if (!PlayerController->GetHitResultAtScreenPosition(FVector2D(ViewportWidth / 2, ViewportHeight / 2), ECC_Visibility, false, Hit))
	{
		return nullptr;
	}

	const float DistanceToPlayer = FVector::Distance(Pawn->GetActorLocation(), Hit.Location);
	if (DistanceToPlayer > MaxDistanceToPlayer)
	{
		return nullptr;
	}

	return Cast<UPrimitiveComponent>(Hit.GetComponent());
}
