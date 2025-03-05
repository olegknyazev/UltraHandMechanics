#include "UHAttachable.h"

#include "UHBlockMovementComponent.h"
#include "Engine/OverlapResult.h"


UUHAttachable::UUHAttachable()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHAttachable::StartAttaching(float InMaxAttachDistance)
{
	if (AttachablePrimitive && !Sockets.IsEmpty())
	{
		bAttachInProgress = true;
		MaxAttachDistance = InMaxAttachDistance;
	}
}

void UUHAttachable::StopAttaching()
{
	bAttachInProgress = false;
	StopSticking();
}

bool UUHAttachable::StartSticking()
{
	if (bAttachInProgress && CurrentTarget && CurrentDistance < MaxAttachDistance && MovementComponent)
	{
		bAttachInProgress = false;
		bStickInProgress = true;
		OurTargetRotation = SnappedRelativeRotation(AttachablePrimitive, CurrentTarget->AttachablePrimitive);
		TheirTargetRotation = SnappedRelativeRotation(CurrentTarget->AttachablePrimitive, AttachablePrimitive);
		AttachablePrimitive->SetSimulatePhysics(false);
		CurrentTarget->AttachablePrimitive->SetSimulatePhysics(false);
		RemainingDistance = -1.f;
		RemainingAngle = -1.f;
		return true;
	}
	
	return false;
}

void UUHAttachable::BeginPlay()
{
	Super::BeginPlay();
}

void UUHAttachable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const FUHAttachmentSocket& Socket : Sockets)
	{
		const FVector Location = GetOwner()->GetActorTransform().TransformPosition(Socket.Location);
		const FRotator Rotation = GetOwner()->GetActorRotation();
		DrawDebugBox(GetWorld(), Location, FVector(3.f), Rotation.Quaternion(), FColor::Red);
	}

	if (bAttachInProgress)
	{
		UpdateCurrentTarget();

		if (CurrentTarget != nullptr)
		{
			const FVector OurSocketLocation = AttachablePrimitive->GetComponentTransform().TransformPosition(Sockets[CurrentOurSocketIndex].Location);
			const FVector TheirSocketLocation = CurrentTarget->AttachablePrimitive->GetComponentTransform().TransformPosition(CurrentTarget->Sockets[CurrentTheirSocketIndex].Location);
			DrawDebugLine(
				GetWorld(),
				OurSocketLocation,
				TheirSocketLocation,
				FColor::Cyan);
		}
	}

	if (bStickInProgress)
	{
		const FVector OurSocketLocation = AttachablePrimitive->GetComponentTransform().TransformPosition(Sockets[CurrentOurSocketIndex].Location);
		const FVector TheirSocketLocation = CurrentTarget->AttachablePrimitive->GetComponentTransform().TransformPosition(CurrentTarget->Sockets[CurrentTheirSocketIndex].Location);
		DrawDebugLine(
			GetWorld(),
			OurSocketLocation,
			TheirSocketLocation,
			FColor::Red);

		const FRotator OurDelta = (OurTargetRotationInWorldSpace() - AttachablePrimitive->GetComponentRotation()).GetNormalized();
		MovementComponent->AngularVelocity = OurDelta * RotationSpeed;
		
		const FRotator TheirDelta = (TheirTargetRotationInWorldSpace() - CurrentTarget->AttachablePrimitive->GetComponentRotation()).GetNormalized();
		CurrentTarget->MovementComponent->AngularVelocity = TheirDelta * RotationSpeed;

		MovementComponent->Velocity = (TheirSocketLocation - OurSocketLocation) * MovementSpeed;
		CurrentTarget->MovementComponent->Velocity = (OurSocketLocation - TheirSocketLocation) * MovementSpeed;

		const float NewRemainingDistance = FVector::Distance(TheirSocketLocation, OurSocketLocation);
		const float NewRemainingAngle = OurDelta.Quaternion().GetAngle();

		if (NewRemainingDistance < 1.f && NewRemainingAngle < 0.01f)
		{
			UE_LOG(LogTemp, Display, TEXT("Sticking complete"));

			StopSticking();
		}
		else if ((RemainingDistance > 0.f && FMath::Abs(NewRemainingDistance - RemainingDistance) < UE_KINDA_SMALL_NUMBER) &&
			(RemainingAngle > 0.f && FMath::Abs(NewRemainingAngle - RemainingAngle) < UE_KINDA_SMALL_NUMBER))
		{
			UE_LOG(LogTemp, Display, TEXT("Stuck!"));
			
			StopSticking();
		}

		RemainingDistance = NewRemainingDistance;
		RemainingAngle = NewRemainingAngle;
	}
}

FRotator UUHAttachable::SnappedRelativeRotation(USceneComponent* Component, USceneComponent* Space)
{
	return Space->GetComponentTransform()
		.InverseTransformRotation(Component->GetComponentRotation().Quaternion())
		.Rotator()
		.GridSnap(FRotator(45.f, 45.f, 45.f));
}

FRotator UUHAttachable::OurTargetRotationInWorldSpace() const
{
	return CurrentTarget->AttachablePrimitive->GetComponentTransform().TransformRotation(OurTargetRotation.Quaternion()).Rotator();
}

FRotator UUHAttachable::TheirTargetRotationInWorldSpace() const
{
	return AttachablePrimitive->GetComponentTransform().TransformRotation(TheirTargetRotation.Quaternion()).Rotator();
}

void UUHAttachable::UpdateCurrentTarget()
{
	const FTransform& AttachableTransform = AttachablePrimitive->GetComponentTransform(); 
		
	const FCollisionShape InflatedShape = AttachablePrimitive->GetCollisionShape(MaxAttachDistance);
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetOwner());
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		AttachablePrimitive->GetComponentLocation(),
		AttachablePrimitive->GetComponentRotation().Quaternion(),
		ECC_Visibility,
		InflatedShape,
		QueryParams);
	
	CurrentOurSocketIndex = -1;
	CurrentTheirSocketIndex = -1;
	CurrentTarget = nullptr;
	CurrentDistance = std::numeric_limits<float>::max();
		
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (auto* const OtherAttachable = Overlap.GetActor()->FindComponentByClass<UUHAttachable>())
		{
			for (int32 TI = 0; TI < OtherAttachable->Sockets.Num(); ++TI)
			{
				const FUHAttachmentSocket& TheirSocket = OtherAttachable->Sockets[TI];
				const FVector TheirSocketLocation = OtherAttachable->AttachablePrimitive->GetComponentTransform().TransformPosition(TheirSocket.Location);
					
				for (int32 OI = 0; OI < Sockets.Num(); ++OI)
				{
					const FUHAttachmentSocket& OurSocket = Sockets[OI];
					const FVector OurSocketLocation = AttachableTransform.TransformPosition(OurSocket.Location);
					const float Distance = FVector::Distance(TheirSocketLocation, OurSocketLocation);

					if (Distance < CurrentDistance)
					{
						CurrentDistance = Distance;
						CurrentOurSocketIndex = OI;
						CurrentTheirSocketIndex = TI;
						CurrentTarget = OtherAttachable;
					}
				}
			}
		}
	}
}

void UUHAttachable::StopSticking()
{
	if (bStickInProgress)
	{
		MovementComponent->StopMovementImmediately();
		CurrentTarget->MovementComponent->StopMovementImmediately();
			
		AttachablePrimitive->SetSimulatePhysics(true);
		CurrentTarget->AttachablePrimitive->SetSimulatePhysics(true);
			
		CurrentOurSocketIndex = -1;
		CurrentTheirSocketIndex = -1;
		CurrentTarget = nullptr;
		CurrentDistance = 0.f;
		TheirTargetRotation = FRotator::ZeroRotator;
		OurTargetRotation = FRotator::ZeroRotator;
		
		bStickInProgress = false;
	}
}
