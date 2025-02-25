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

	if (auto* const PlayerController = Cast<APlayerController>(GetOwner()))
	{
		int32 ViewportWidth, ViewportHeight;
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

		UUHBlock* BlockUnderAim = nullptr;
		FHitResult Hit;
		if (PlayerController->GetHitResultAtScreenPosition(FVector2D(ViewportWidth / 2, ViewportHeight / 2), ECC_Visibility, false, Hit))
		{
			BlockUnderAim = Hit.GetActor()->FindComponentByClass<UUHBlock>();
		}
	
		if (SelectedBlock != BlockUnderAim)
		{
			SelectedBlock = BlockUnderAim;
			UE_LOG(LogTemp, Warning, TEXT("SelectedBlock is now %s"), SelectedBlock ? *SelectedBlock->GetName() : TEXT("None"));
		}
	}
}
