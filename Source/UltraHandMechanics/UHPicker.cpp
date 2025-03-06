#include "UHPicker.h"

#include "UHBaseBlock.h"


UUHPicker::UUHPicker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

AUHBaseBlock* UUHPicker::GetSelectedBlock() const
{
	return SelectedBlock;
}

UStaticMeshComponent* UUHPicker::GetSelectedPart() const
{
	return SelectedMesh;
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

void UUHPicker::BeginPlay()
{
	Super::BeginPlay();
}

void UUHPicker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPickingEnabled)
	{
		SetSelectedPart(TraceMeshUnderAim());
	}
}

void UUHPicker::SetSelectedPart(UStaticMeshComponent* Part)
{
	auto* const Block = Part ? Cast<AUHBaseBlock>(Part->GetOwner()) : nullptr;
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

	if (bBlockChanged || SelectedMesh != Part)
	{
		if (SelectedBlock)
		{
			SelectedBlock->SetHighlightedPart(Part);
		}
		
		SelectedMesh = Part;

		UE_LOG(LogTemp, Display, TEXT("SelectedBlock is now %s (%s)"),
        	SelectedBlock ? *SelectedBlock->GetName() : TEXT("None"),
        	SelectedMesh ? *SelectedMesh->GetName() : TEXT("None"));
	}
}

UStaticMeshComponent* UUHPicker::TraceMeshUnderAim() const
{
	auto* const PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController)
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

	return Cast<UStaticMeshComponent>(Hit.GetComponent());
}
