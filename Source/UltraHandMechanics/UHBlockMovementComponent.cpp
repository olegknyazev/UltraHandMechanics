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

	const FRotator RotationDelta = ClampAngularVelocity(AngularVelocity) * DeltaTime;
	const FVector RotationInducedOffset = RotationDelta.IsNearlyZero() ? FVector::Zero() : RotationDelta.Vector() * DeltaTime * 10.f;
	const FVector MoveDelta = Velocity * DeltaTime + RotationInducedOffset;

	if (!MoveDelta.IsNearlyZero() || !RotationDelta.IsNearlyZero())
	{
		const FRotator Rotation = UpdatedComponent->GetComponentRotation() + RotationDelta;
		
		FHitResult Hit;
		SafeMoveUpdatedComponent(MoveDelta, Rotation, true, Hit);
		if (Hit.bBlockingHit)
		{
			SlideAlongSurface(MoveDelta, 1 - Hit.Time, Hit.Normal, Hit, false);
		}
	}
}

void UUHBlockMovementComponent::StopMovementImmediately()
{
	Super::StopMovementImmediately();
	
	AngularVelocity = FRotator::ZeroRotator;
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

FRotator UUHBlockMovementComponent::ClampAngularVelocity(const FRotator& AngularVelocity) const
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

	const float TangentialVelocity = BlockRadius * 2.f * FMath::Sin(FQuat::Identity.AngularDistance(AngularVelocity.Quaternion()) / 2.f);
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

