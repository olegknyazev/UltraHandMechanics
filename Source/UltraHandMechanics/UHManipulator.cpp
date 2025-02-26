#include "UHManipulator.h"

#include "UHBlock.h"

UUHManipulator::UUHManipulator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHManipulator::StartManipulation(UUHBlock* Block)
{
	StopManipulation();
	
	BlockBeingManipulated = Block;

	if (BlockBeingManipulated)
	{
		if (auto* const BlockPrimitive = BlockBeingManipulated->GetPrimitiveComponent())
		{
			const auto OriginTransform = GetOriginTransform();
			BlockRelativeLocation = OriginTransform.InverseTransformPosition(BlockPrimitive->GetComponentLocation());
			BlockRelativeRotation = OriginTransform.InverseTransformRotation(BlockPrimitive->GetComponentRotation().Quaternion());
		}
		else
		{
			BlockRelativeLocation = FVector::Zero();
			BlockRelativeRotation = FQuat::Identity;
		}
	}
}

void UUHManipulator::StopManipulation()
{
	BlockBeingManipulated = nullptr;
}

void UUHManipulator::BeginPlay()
{
	Super::BeginPlay();
}

void UUHManipulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (BlockBeingManipulated)
	{
		if (auto* const BlockPrimitive = BlockBeingManipulated->GetPrimitiveComponent())
		{
			const auto OriginTransform = GetOriginTransform();
			BlockPrimitive->SetWorldLocation(OriginTransform.TransformPosition(BlockRelativeLocation), false);
			BlockPrimitive->SetWorldRotation(OriginTransform.TransformRotation(BlockRelativeRotation), false);
		}
	}
}

FTransform UUHManipulator::GetOriginTransform() const
{
	auto* const Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return FTransform::Identity;
	}
	
	FRotator Rotation = Pawn->GetControlRotation();
	Rotation.Pitch = 0.f;
	return FTransform(Rotation, GetComponentLocation());
}

