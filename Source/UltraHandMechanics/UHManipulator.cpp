#include "UHManipulator.h"

#include "UHAttachable.h"
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
		const FTransform OriginTransform = GetOriginTransform();
		BlockRelativeLocation = OriginTransform.InverseTransformPosition(BlockBeingManipulated->GetBlockLocation());
		BlockRelativeCurrentRotation = OriginTransform.InverseTransformRotation(BlockBeingManipulated->GetBlockRotation().Quaternion());
		BlockRelativeTargetRotation = SnapRotation(BlockRelativeCurrentRotation);

		BlockBeingManipulated->SetManipulated(true);

		if (auto* const Attachable = BlockBeingManipulated->GetOwner()->FindComponentByClass<UUHAttachable>())
		{
			Attachable->StartAttaching(MaxAttachDistance);
		}
	}
}

void UUHManipulator::StopManipulation()
{
	if (BlockBeingManipulated)
	{
		BlockBeingManipulated->SetManipulated(false);
		
		if (auto* const Attachable = BlockBeingManipulated->GetOwner()->FindComponentByClass<UUHAttachable>())
		{
			Attachable->StopAttaching();
		}
	}
	
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

bool UUHManipulator::StartSticking()
{
	if (BlockBeingManipulated)
	{
		if (BlockBeingManipulated->StartSticking())
		{
			BlockBeingManipulated = nullptr;
			return true;
		}
	}

	return false;
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

		const FTransform OriginTransform = GetOriginTransform();
		const FVector TargetLocation = OriginTransform.TransformPosition(BlockRelativeLocation);
		const FQuat TargetRotation = OriginTransform.TransformRotation(BlockRelativeCurrentRotation);
		BlockBeingManipulated->SetTargetPlacement(TargetLocation, TargetRotation.Rotator());
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
