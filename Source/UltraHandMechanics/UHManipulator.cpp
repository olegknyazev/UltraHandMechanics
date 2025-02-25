#include "UHManipulator.h"

UUHManipulator::UUHManipulator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHManipulator::BeginPlay()
{
	Super::BeginPlay();
}

void UUHManipulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

