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
			BlockRelativeCurrentRotation = OriginTransform.InverseTransformRotation(BlockPrimitive->GetComponentRotation().Quaternion());
			BlockRelativeTargetRotation = SnapRotation(BlockRelativeCurrentRotation);
		}
		else
		{
			BlockRelativeLocation = FVector::Zero();
			BlockRelativeCurrentRotation = FQuat::Identity;
			BlockRelativeTargetRotation = FQuat::Identity;
		}
	}
}

void UUHManipulator::StopManipulation()
{
	BlockBeingManipulated = nullptr;
}

void UUHManipulator::MoveRelative(const FVector& Offset)
{
	if (BlockBeingManipulated)
	{
		BlockRelativeLocation += Offset;
	}
}

void UUHManipulator::TurnLeft()
{
	if (BlockBeingManipulated)
	{
		const FQuat Turn{FVector::ZAxisVector, FMath::DegreesToRadians(SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnRight()
{
	if (BlockBeingManipulated)
	{
		const FQuat Turn{FVector::ZAxisVector, FMath::DegreesToRadians(-SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnUp()
{
	if (BlockBeingManipulated)
	{
		const FQuat Turn{FVector::YAxisVector, FMath::DegreesToRadians(SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnDown()
{
	if (BlockBeingManipulated)
	{
		const FQuat Turn{FVector::YAxisVector, FMath::DegreesToRadians(-SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
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
		BlockRelativeCurrentRotation = FMath::QInterpTo(BlockRelativeCurrentRotation, BlockRelativeTargetRotation, DeltaTime, BlockRotationSpeed);
		
		if (auto* const BlockPrimitive = BlockBeingManipulated->GetPrimitiveComponent())
		{
			const auto OriginTransform = GetOriginTransform();
			BlockPrimitive->SetWorldLocation(OriginTransform.TransformPosition(BlockRelativeLocation), false);
			BlockPrimitive->SetWorldRotation(OriginTransform.TransformRotation(BlockRelativeCurrentRotation), false);
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

FRotator UUHManipulator::SnapRotation(const FRotator& Rotation) const
{
	return Rotation.GridSnap(FRotator{SnapDegree, SnapDegree, SnapDegree});
}

FQuat UUHManipulator::SnapRotation(const FQuat& Rotation) const
{
	return SnapRotation(Rotation.Rotator()).Quaternion();
}
