#include "UHManipulator.h"

#include "UHBaseBlock.h"
#include "UHBlockMovementComponent.h"


namespace ManipulatorCVars
{
	static TAutoConsoleVariable<bool> DebugDrawManipulation(
		TEXT("uh.DebugDrawManipulation"),
		true,
		TEXT("Defines whether the manipulation debug visualization is enabled."),
		ECVF_Default);
}


UUHManipulator::UUHManipulator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UUHManipulator::IsManipulating() const
{
	return ManipulatedBlock != nullptr;
}

void UUHManipulator::StartManipulation(UStaticMeshComponent* InManipulatedPart)
{
	StopManipulation();

	ManipulatedBlockPart = InManipulatedPart;
	ManipulatedBlock = ManipulatedBlockPart ? Cast<AUHBaseBlock>(ManipulatedBlockPart->GetOwner()) : nullptr;

	if (ManipulatedBlock)
	{
		ManipulatedBlock->Reroot(ManipulatedBlockPart);
		
		const FTransform OriginTransform = GetOriginTransform();
		BlockRelativeLocation = ClampOffset(OriginTransform.InverseTransformPosition(ManipulatedBlock->GetActorLocation()));
		BlockRelativeCurrentRotation = OriginTransform.InverseTransformRotation(ManipulatedBlock->GetActorRotation().Quaternion());
		BlockRelativeTargetRotation = SnapRotation(BlockRelativeCurrentRotation);

		ManipulatedBlock->SetManipulated(true);
		ManipulatedBlock->StartAttaching(MaxAttachDistance);
	}
}

void UUHManipulator::StopManipulation()
{
	if (ManipulatedBlock)
	{
		ManipulatedBlock->SetManipulated(false);
		ManipulatedBlock->StopAttaching();
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
			ManipulatedBlockPart = nullptr;
			return true;
		}
	}

	return false;
}

void UUHManipulator::Detach()
{
	if (ManipulatedBlock)
	{
		ManipulatedBlock->Detach(ManipulatedBlockPart);
	}
}

FVector UUHManipulator::GetOffset() const
{
	return ManipulatedBlock ? GetOriginTransform().InverseTransformPosition(ManipulatedBlock->GetActorLocation()) : FVector::Zero();
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
		
		if (TargetApproachTime > 0.f)
		{
			const FVector TargetLinearVelocity = (TargetLocation - ManipulatedBlock->GetActorLocation()) / TargetApproachTime;

			ManipulatedBlock->MovementComponent->Velocity = TargetLinearVelocity;

			const FQuat CurrentRotation = ManipulatedBlock->GetActorRotation().Quaternion();
			FQuat AlignedTargetRotation = TargetRotation;
			AlignedTargetRotation.EnforceShortestArcWith(CurrentRotation);
			const FVector TargetAngularVelocity = (AlignedTargetRotation * CurrentRotation.Inverse()).ToRotationVector() / TargetApproachTime;

			ManipulatedBlock->MovementComponent->AngularVelocity = TargetAngularVelocity;
		}
		
		if (ManipulatorCVars::DebugDrawManipulation.GetValueOnGameThread())
		{
			DrawDebugSphere(GetWorld(), TargetLocation, 25.f, 16, FColor::Emerald);
			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), TargetLocation, FColor::Emerald);

			DrawDebugSphere(GetWorld(), ManipulatedBlock->GetActorLocation(), 10.f, 12, FColor::Red);
			DrawDebugLine(GetWorld(), ManipulatedBlock->GetActorLocation(), TargetLocation, FColor::Red);
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

FVector UUHManipulator::ClampOffset(const FVector& Offset) const
{
	FVector Clamped = Offset.GetClampedToMaxSize2D(MaxHorizontalOffset);
	Clamped.Z = FMath::Clamp(Clamped.Z, -MaxVerticalOffset, +MaxVerticalOffset);
	return Clamped;
}
