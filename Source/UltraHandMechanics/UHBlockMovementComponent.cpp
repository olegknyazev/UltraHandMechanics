#include "UHBlockMovementComponent.h"

#include "Algo/MaxElement.h"


UUHBlockMovementComponent::UUHBlockMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHBlockMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateBlockRadius();
}

void UUHBlockMovementComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdatedPrimitive)
	{
		return;
	}

	if (!Velocity.IsNearlyZero())
	{
		const FVector TargetLinearVelocity = Velocity;
		const FVector LinearVelocityChange = TargetLinearVelocity - UpdatedPrimitive->GetPhysicsLinearVelocity();
		const FVector LinearAccelerationChange = LinearVelocityChange * 20.f;
		const FVector ClampedLinearAccelerationChange =
			MaxLinearForce > 0.f
				? LinearAccelerationChange.GetClampedToMaxSize(MaxLinearForce)
				: LinearAccelerationChange;

		UpdatedPrimitive->AddForce(ClampedLinearAccelerationChange, NAME_None, true);
	}

	if (!AngularVelocity.IsNearlyZero())
	{
		const FVector TargetAngularVelocity = ClampAngularVelocity(AngularVelocity);
		const FVector AngularVelocityChange = TargetAngularVelocity - UpdatedPrimitive->GetPhysicsAngularVelocityInRadians();
		const FVector AngularAccelerationChange = AngularVelocityChange * 20.f;

		UpdatedPrimitive->AddTorqueInRadians(AngularAccelerationChange, NAME_None, true);
	}
}

void UUHBlockMovementComponent::StopMovementImmediately()
{
	Super::StopMovementImmediately();
	
	AngularVelocity = FVector::Zero();
}

void UUHBlockMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	UpdateBlockRadius();
}

void UUHBlockMovementComponent::UpdatedComponentShapeMightChange()
{
	UpdateBlockRadius();
}

FVector UUHBlockMovementComponent::ClampAngularVelocity(const FVector& AngularVelocity) const
{
	if (MaxAngularSpeed <= 0.f)
	{
		return AngularVelocity;
	}

	if (!UpdatedComponent)
	{
		return AngularVelocity;
	}

	if (AngularVelocity.IsZero())
	{
		return AngularVelocity;
	}

	const float TangentialVelocity = BlockRadius * AngularVelocity.Length();
	const float ScaleFactor = FMath::Clamp(MaxAngularSpeed / TangentialVelocity, 0.f, 1.f);
	return AngularVelocity * ScaleFactor;
}

void UUHBlockMovementComponent::UpdateBlockRadius()
{
	if (!UpdatedComponent)
	{
		BlockRadius = 0.f;
		return;
	}
	
	FVector Vertices[8];
	const FBox ComponentsBounds = UpdatedComponent->GetOwner()->CalculateComponentsBoundingBoxInLocalSpace();
	ComponentsBounds.GetVertices(Vertices);
	const FVector* FarthestVertex = Algo::MaxElementBy(Vertices, [](const FVector& V) { return V.SizeSquared(); });
	BlockRadius = FarthestVertex ? FarthestVertex->Size() : 0.f;
}

