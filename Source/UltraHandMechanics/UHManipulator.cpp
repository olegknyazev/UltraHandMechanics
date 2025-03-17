#include "UHManipulator.h"

#include "UHAttachable.h"
#include "UHBaseBlock.h"
#include "UHBlock.h"

UUHManipulator::UUHManipulator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UUHManipulator::IsManipulationInProgress() const
{
	return ManipulatedBlock != nullptr;
}

void UUHManipulator::StartManipulation(UStaticMeshComponent* InManipulatedPart)
{
	StopManipulation();

	ManipulatedPart = InManipulatedPart;
	ManipulatedBlock = ManipulatedPart ? ManipulatedPart->GetOwner()->FindComponentByClass<UUHBlock>() : nullptr;

	if (ManipulatedBlock)
	{
		Cast<AUHBaseBlock>(ManipulatedBlock->GetOwner())->Reroot(ManipulatedPart);
		
		const FTransform OriginTransform = GetOriginTransform();
		BlockRelativeLocation = ClampOffset(OriginTransform.InverseTransformPosition(ManipulatedBlock->GetBlockLocation()));
		BlockRelativeCurrentRotation = OriginTransform.InverseTransformRotation(ManipulatedBlock->GetBlockRotation().Quaternion());
		BlockRelativeTargetRotation = SnapRotation(BlockRelativeCurrentRotation);

		ManipulatedBlock->SetManipulated(true);

		if (auto* const Attachable = ManipulatedBlock->GetOwner()->FindComponentByClass<UUHAttachable>())
		{
			Attachable->StartAttaching(MaxAttachDistance);
		}
	}
}

void UUHManipulator::StopManipulation()
{
	if (ManipulatedBlock)
	{
		ManipulatedBlock->SetManipulated(false);
		
		if (auto* const Attachable = ManipulatedBlock->GetOwner()->FindComponentByClass<UUHAttachable>())
		{
			Attachable->StopAttaching();
		}
	}
	
	ManipulatedBlock = nullptr;
}

void UUHManipulator::MoveRelative(const FVector& Offset)
{
	if (ManipulatedBlock)
	{
		BlockRelativeLocation = ClampOffset(BlockRelativeLocation + Offset);
	}
}

void UUHManipulator::TurnLeft()
{
	if (ManipulatedBlock)
	{
		const FQuat Turn{FVector::ZAxisVector, FMath::DegreesToRadians(SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnRight()
{
	if (ManipulatedBlock)
	{
		const FQuat Turn{FVector::ZAxisVector, FMath::DegreesToRadians(-SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnUp()
{
	if (ManipulatedBlock)
	{
		const FQuat Turn{FVector::YAxisVector, FMath::DegreesToRadians(SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

void UUHManipulator::TurnDown()
{
	if (ManipulatedBlock)
	{
		const FQuat Turn{FVector::YAxisVector, FMath::DegreesToRadians(-SnapDegree * 1.25f)};
		BlockRelativeTargetRotation = SnapRotation(Turn * BlockRelativeTargetRotation);
	}
}

bool UUHManipulator::StartSticking()
{
	if (ManipulatedBlock)
	{
		if (ManipulatedBlock->StartSticking())
		{
			ManipulatedBlock = nullptr;
			return true;
		}
	}

	return false;
}

void UUHManipulator::Detach()
{
	if (ManipulatedBlock)
	{
		if (auto* const Attachable = ManipulatedBlock->GetOwner()->FindComponentByClass<UUHAttachable>())
		{
			Attachable->Detach(ManipulatedPart);
		}

		ManipulatedBlock->SetManipulated(false);
	}
}

FVector UUHManipulator::GetOffset() const
{
	return ManipulatedBlock ? GetOriginTransform().InverseTransformPosition(ManipulatedBlock->GetBlockLocation()) : FVector::Zero();
}

FVector UUHManipulator::GetError() const
{
	return ManipulatedBlock ? BlockRelativeLocation - GetOffset() : FVector::Zero();
}

void UUHManipulator::BeginPlay()
{
	Super::BeginPlay();
}

void UUHManipulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ManipulatedBlock)
	{
		BlockRelativeCurrentRotation = FMath::QInterpTo(BlockRelativeCurrentRotation, BlockRelativeTargetRotation, DeltaTime, BlockRotationSpeed);

		const FTransform OriginTransform = GetOriginTransform();
		const FVector TargetLocation = OriginTransform.TransformPosition(BlockRelativeLocation);
		const FQuat TargetRotation = OriginTransform.TransformRotation(BlockRelativeCurrentRotation);
		ManipulatedBlock->SetTargetPlacement(TargetLocation, TargetRotation.Rotator());

		DrawDebugSphere(GetWorld(), TargetLocation, 25.f, 16, FColor::Emerald);
		DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), TargetLocation, FColor::Emerald);

		DrawDebugSphere(GetWorld(), ManipulatedBlock->GetBlockLocation(), 10.f, 12, FColor::Red);
		DrawDebugLine(GetWorld(), ManipulatedBlock->GetBlockLocation(), TargetLocation, FColor::Red);
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

FVector UUHManipulator::ClampOffset(const FVector& Offset) const
{
	FVector Clamped = Offset.GetClampedToMaxSize2D(MaxHorizontalOffset);
	Clamped.Z = FMath::Clamp(Clamped.Z, -MaxVerticalOffset, +MaxVerticalOffset);
	return Clamped;
}
