#include "UHPicker.h"

#include "UHBlock.h"


UUHPicker::UUHPicker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHPicker::BeginPlay()
{
	Super::BeginPlay();
}

void UUHPicker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetSelectedBlock(TraceBlockUnderAim());
}

void UUHPicker::SetSelectedBlock(UUHBlock* Block)
{
	if (SelectedBlock != Block)
	{
		if (SelectedBlock)
		{
			SelectedBlock->SetHighlighted(false);
		}

		SelectedBlock = Block;

		if (SelectedBlock)
		{
			SelectedBlock->SetHighlighted(true);
		}
			
		UE_LOG(LogTemp, Display, TEXT("SelectedBlock is now %s"), SelectedBlock ? *SelectedBlock->GetName() : TEXT("None"));
	}
}

UUHBlock* UUHPicker::TraceBlockUnderAim() const
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
	
	return Hit.GetActor()->FindComponentByClass<UUHBlock>();
}
