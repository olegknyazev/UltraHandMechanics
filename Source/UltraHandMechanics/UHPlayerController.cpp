#include "UHPlayerController.h"

#include "UHPicker.h"

AUHPlayerController::AUHPlayerController()
{
	Picker = CreateDefaultSubobject<UUHPicker>(TEXT("Picker"));
}

void AUHPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
