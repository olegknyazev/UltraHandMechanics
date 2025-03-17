#include "UHAttachable.h"

#include "UHBaseBlock.h"
#include "UHBlockMovementComponent.h"
#include "UHDebugDrawFunctions.h"
#include "Engine/OverlapResult.h"


DEFINE_LOG_CATEGORY(LogUHAttachable);


UUHAttachable::UUHAttachable()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

bool UUHAttachable::IsAttachInProgress() const
{
	return State == EUHAttachableState::Attaching;
}

void UUHAttachable::StartAttaching(float InMaxAttachDistance)
{
	if (!Parts.IsEmpty())
	{
		State = EUHAttachableState::Attaching;
		MaxAttachDistance = InMaxAttachDistance;
	}
}

void UUHAttachable::StopAttaching()
{
	StopSticking();
	State = EUHAttachableState::Idle;
}

bool UUHAttachable::IsStickInProgress() const
{
	return State == EUHAttachableState::Sticking;
}

bool UUHAttachable::StartSticking()
{
	if (IsAttachInProgress() && CurrentTarget && CurrentDistance < MaxAttachDistance && MovementComponent)
	{
		ensure(CurrentTheirPrimitive);
		ensure(CurrentOurPrimitive);

		auto* OurBlock = Cast<AUHBaseBlock>(CurrentOurPrimitive->GetOwner());
		if (!ensure(OurBlock))
		{
			return false;
		}
		
		OurBlock->Reroot(CurrentOurPrimitive);
		
		State = EUHAttachableState::Sticking;
		OurTargetRotation = SnappedRelativeRotation(CurrentOurPrimitive, CurrentTheirPrimitive);
		TheirTargetRotation = SnappedRelativeRotation(CurrentTheirPrimitive, CurrentOurPrimitive);
		RemainingDistance = -1.f;
		RemainingAngle = -1.f;
		
		return true;
	}
	
	return false;
}

void UUHAttachable::Detach(USceneComponent* PartToDetach)
{
	if (!ensure(PartToDetach == GetOwner()->GetRootComponent()))
	{
		return;
	}
	
	const FUHAttachmentPart* Part = Parts.FindByPredicate([PartToDetach](const FUHAttachmentPart& P)
	{
		return P.PrimitiveComponent == PartToDetach;
	});

	if (!ensure(Part))
	{
		return;
	}

	for (const uint32 Index : Part->ConnectedPartIndices)
	{
		const FUHAttachmentPart& AttachedPart = Parts[Index];
		const UStaticMeshComponent* DetachedMesh = Cast<UStaticMeshComponent>(AttachedPart.PrimitiveComponent);

		auto* Actor = GetWorld()->SpawnActor<AUHBaseBlock>(
			GetOwner()->GetClass(),
			AttachedPart.PrimitiveComponent->GetComponentLocation(),
			AttachedPart.PrimitiveComponent->GetComponentRotation());

		Actor->MeshComponent->SetStaticMesh(DetachedMesh->GetStaticMesh());
		Actor->MeshComponent->SetMaterial(0, DetachedMesh->GetMaterial(0));

		TSet<uint32> VisitedIndices;
		VisitedIndices.Add(Part - Parts.GetData());

		ensure(Actor->AttachableComponent->Parts.Num() == 1);
		ensure(Actor->AttachableComponent->Parts[0].PrimitiveComponent == Actor->MeshComponent);
		Actor->AttachableComponent->CopyPartsFrom(this, Index, 0, VisitedIndices);
	}

	for (const FUHAttachmentPart& P : Parts)
	{
		if (P.PrimitiveComponent != PartToDetach)
		{
			P.PrimitiveComponent->DestroyComponent();
		}
	}

	Parts.RemoveAll([PartToDetach](const FUHAttachmentPart& P) { return P.PrimitiveComponent != PartToDetach; });

	ensure(Parts.Num() == 1);
	Parts[0].ConnectedPartIndices.Reset();
	
	MovementComponent->UpdatedComponentShapeMightChange();
}

void UUHAttachable::CopyPartsFrom(
	const UUHAttachable* Other,
	uint32 OtherParentPartIndex,
	uint32 OurParentPartIndex,
	TSet<uint32>& CopiedParts)
{
	const FUHAttachmentPart& OurParentPart = Parts[OurParentPartIndex];
	const FUHAttachmentPart& OtherParentPart = Other->Parts[OtherParentPartIndex];
	for (const uint32 OtherIndex : OtherParentPart.ConnectedPartIndices)
	{
		bool bAlreadyCopied = false;
		CopiedParts.Add(OtherIndex, &bAlreadyCopied);
		if (!bAlreadyCopied)
		{
			const FUHAttachmentPart& OtherPart = Other->Parts[OtherIndex];
			const auto* OtherMeshComponent = Cast<UStaticMeshComponent>(OtherPart.PrimitiveComponent);
			auto* NewMeshComponent = NewObject<UStaticMeshComponent>(GetOwner());
			NewMeshComponent->SetupAttachment(OurParentPart.PrimitiveComponent);
			NewMeshComponent->SetWorldLocation(OtherMeshComponent->GetComponentLocation());
			NewMeshComponent->SetWorldRotation(OtherMeshComponent->GetComponentRotation());
			NewMeshComponent->SetStaticMesh(OtherMeshComponent->GetStaticMesh());
			NewMeshComponent->SetMaterial(0, OtherMeshComponent->GetMaterial(0));
			NewMeshComponent->RegisterComponent();
			NewMeshComponent->WeldTo(OurParentPart.PrimitiveComponent);

			const uint32 AddedIndex = Parts.Emplace(NewMeshComponent, OtherPart.Sockets);
			Parts[AddedIndex].ConnectedPartIndices.Add(OurParentPartIndex);
			Parts[OurParentPartIndex].ConnectedPartIndices.Add(AddedIndex);
			
			CopyPartsFrom(Other, OtherIndex, AddedIndex, CopiedParts);
		}
	}
}

void UUHAttachable::BeginPlay()
{
	Super::BeginPlay();

	if (AttachablePrimitive && !Sockets.IsEmpty())
	{
		Parts.Emplace(AttachablePrimitive, Sockets);
	}
}

void UUHAttachable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const FUHAttachmentPart& Part : Parts)
	{
		const FTransform& Transform = Part.PrimitiveComponent->GetComponentTransform();
		for (const FUHAttachmentSocket& Socket : Part.Sockets)
		{
			const FVector Location = Transform.TransformPosition(Socket.Location);
			const FRotator Rotation = Part.PrimitiveComponent->GetComponentRotation();
			DrawDebugBox(GetWorld(), Location, FVector(3.f), Rotation.Quaternion(), FColor::Red);
		}
	}

	if (IsAttachInProgress())
	{
		UpdateCurrentTarget();

		if (CurrentTarget != nullptr)
		{
			const FVector OurSocketLocation = CurrentOurPrimitive->GetComponentTransform().TransformPosition(Parts[CurrentOurPartIndex].Sockets[CurrentOurSocketIndex].Location);
			const FVector TheirSocketLocation = CurrentTheirPrimitive->GetComponentTransform().TransformPosition(CurrentTarget->Parts[CurrentTheirPartIndex].Sockets[CurrentTheirSocketIndex].Location);
			DrawDebugLine(
				GetWorld(),
				OurSocketLocation,
				TheirSocketLocation,
				FColor::Cyan);
		}
	}

	if (IsStickInProgress())
	{
		const FVector OurSocketLocation = CurrentOurPrimitive->GetComponentTransform().TransformPosition(Parts[CurrentOurPartIndex].Sockets[CurrentOurSocketIndex].Location);
		const FVector TheirSocketLocation = CurrentTheirPrimitive->GetComponentTransform().TransformPosition(CurrentTarget->Parts[CurrentTheirPartIndex].Sockets[CurrentTheirSocketIndex].Location);
		DrawDebugLine(
			GetWorld(),
			OurSocketLocation,
			TheirSocketLocation,
			FColor::Red);

		const FVector OurDelta = (OurTargetRotationInWorldSpace() - CurrentOurPrimitive->GetComponentRotation()).GetNormalized().Quaternion().ToRotationVector();
		MovementComponent->AngularVelocity = OurDelta * RotationSpeed;
		
		const FVector TheirDelta = (TheirTargetRotationInWorldSpace() - CurrentTheirPrimitive->GetComponentRotation()).GetNormalized().Quaternion().ToRotationVector();
		CurrentTarget->MovementComponent->AngularVelocity = TheirDelta * RotationSpeed;

		MovementComponent->Velocity = (TheirSocketLocation - OurSocketLocation) * MovementSpeed;
		CurrentTarget->MovementComponent->Velocity = (OurSocketLocation - TheirSocketLocation) * MovementSpeed;

		const float NewRemainingDistance = FVector::Distance(TheirSocketLocation, OurSocketLocation);
		const float NewRemainingAngle = OurDelta.Size();

		if (NewRemainingDistance < 1.f && NewRemainingAngle < 0.01f)
		{
			UE_LOG(LogUHAttachable, Display, TEXT("Sticking complete"));

			Attach(CurrentTarget);

			StopSticking();
		}
		else if ((RemainingDistance > 0.f && FMath::Abs(NewRemainingDistance - RemainingDistance) < UE_KINDA_SMALL_NUMBER) &&
			(RemainingAngle > 0.f && FMath::Abs(NewRemainingAngle - RemainingAngle) < UE_KINDA_SMALL_NUMBER))
		{
			UE_LOG(LogUHAttachable, Display, TEXT("Stuck!"));
			
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
	return CurrentTheirPrimitive->GetComponentTransform().TransformRotation(OurTargetRotation.Quaternion()).Rotator();
}

FRotator UUHAttachable::TheirTargetRotationInWorldSpace() const
{
	return CurrentOurPrimitive->GetComponentTransform().TransformRotation(TheirTargetRotation.Quaternion()).Rotator();
}

void UUHAttachable::UpdateCurrentTarget()
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(GetOwner());
	
	TArray<FOverlapResult> Overlaps;
	TArray<FOverlapResult> OverlapsLocal;
	for (const FUHAttachmentPart& Part : Parts)
	{
		const FCollisionShape InflatedShape = Part.PrimitiveComponent->GetCollisionShape(MaxAttachDistance);
		const FVector ShapeLocation = Part.PrimitiveComponent->GetComponentLocation();
		const FQuat ShapeRotation = Part.PrimitiveComponent->GetComponentRotation().Quaternion();
		DrawDebugCollisionShape(GetWorld(), InflatedShape, ShapeLocation, ShapeRotation, FColor::Orange);
		OverlapsLocal.Reset();
		GetWorld()->OverlapMultiByChannel(
			OverlapsLocal,
			ShapeLocation,
			ShapeRotation,
			ECC_Visibility,
			InflatedShape,
			QueryParams);
		Overlaps.Append(OverlapsLocal);
	}

	CurrentOurPrimitive = nullptr;
	CurrentTheirPrimitive = nullptr;
	CurrentOurSocketIndex = -1;
	CurrentTheirSocketIndex = -1;
	CurrentTarget = nullptr;
	CurrentDistance = std::numeric_limits<float>::max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (auto* const OtherAttachable = Overlap.GetActor()->FindComponentByClass<UUHAttachable>())
		{
			for (int32 TPI = 0; TPI < OtherAttachable->Parts.Num(); ++TPI)
			{
				const FUHAttachmentPart& TheirPart = OtherAttachable->Parts[TPI];
				for (int32 TSI = 0; TSI < OtherAttachable->Parts[TPI].Sockets.Num(); ++TSI)
				{
					const FUHAttachmentSocket& TheirSocket = TheirPart.Sockets[TSI];
					const FVector TheirSocketLocation = TheirPart.PrimitiveComponent->GetComponentTransform().TransformPosition(TheirSocket.Location);

					for (int32 OPI = 0; OPI < Parts.Num(); ++OPI)
					{
						const FUHAttachmentPart& OurPart = Parts[OPI];
						for (int32 OSI = 0; OSI < Parts[OPI].Sockets.Num(); ++OSI)
						{
							const FUHAttachmentSocket& OurSocket = OurPart.Sockets[OSI];
							const FVector OurSocketLocation = OurPart.PrimitiveComponent->GetComponentTransform().TransformPosition(OurSocket.Location);
							const float Distance = FVector::Distance(TheirSocketLocation, OurSocketLocation);

							if (Distance < CurrentDistance)
							{
								CurrentDistance = Distance;
								CurrentOurPrimitive = OurPart.PrimitiveComponent;
								CurrentTheirPrimitive = TheirPart.PrimitiveComponent;
								CurrentOurPartIndex = OPI;
								CurrentTheirPartIndex = TPI;
								CurrentOurSocketIndex = OSI;
								CurrentTheirSocketIndex = TSI;
								CurrentTarget = OtherAttachable;
							}
						}
					}
				}
			}
		}
	}
}

void UUHAttachable::StopSticking()
{
	if (IsStickInProgress())
	{
		MovementComponent->StopMovementImmediately();
		CurrentTarget->MovementComponent->StopMovementImmediately();

		CurrentOurPrimitive = nullptr;
		CurrentTheirPrimitive = nullptr;
		CurrentOurPartIndex = -1;
		CurrentTheirPartIndex = -1;
		CurrentOurSocketIndex = -1;
		CurrentTheirSocketIndex = -1;
		CurrentTarget = nullptr;
		CurrentDistance = 0.f;
		TheirTargetRotation = FRotator::ZeroRotator;
		OurTargetRotation = FRotator::ZeroRotator;
		
		State = EUHAttachableState::Attaching;
	}
}

void UUHAttachable::Attach(UUHAttachable* Other)
{
	if (!ensure(CurrentOurPrimitive == GetOwner()->GetRootComponent()))
	{
		return;
	}

	const FUHAttachmentPart* OtherStartPart = Other->Parts.FindByPredicate([this](const FUHAttachmentPart& P)
	{
		return P.PrimitiveComponent == CurrentTheirPrimitive;
	});

	if (!ensure(OtherStartPart))
	{
		return;
	}

	TSet<const FUHAttachmentPart*> AttachedParts;
	Attach(Other, OtherStartPart, AttachedParts, CurrentOurPartIndex);

	MovementComponent->UpdatedComponentShapeMightChange();
	
	Other->GetOwner()->Destroy();
}

uint32 UUHAttachable::Attach(
	UUHAttachable* Other,
	const FUHAttachmentPart* OtherPart,
	TSet<const FUHAttachmentPart*>& AttachedParts,
	uint32 OurParentPartIndex)
{
	auto* AttachParent = Parts[OurParentPartIndex].PrimitiveComponent;
	auto* OtherMeshComponent = Cast<UStaticMeshComponent>(OtherPart->PrimitiveComponent);
	
	auto* NewMeshComponent = NewObject<UStaticMeshComponent>(GetOwner());
	NewMeshComponent->SetupAttachment(AttachParent);
	NewMeshComponent->SetWorldLocation(OtherMeshComponent->GetComponentLocation());
	NewMeshComponent->SetWorldRotation(OtherMeshComponent->GetComponentRotation());
	NewMeshComponent->SetStaticMesh(OtherMeshComponent->GetStaticMesh());
	NewMeshComponent->SetMaterial(0, OtherMeshComponent->GetMaterial(0));
	NewMeshComponent->RegisterComponent();
	NewMeshComponent->WeldTo(AttachParent);

	AttachedParts.Add(OtherPart);

	const uint32 AddedPartIndex = Parts.Emplace(NewMeshComponent, OtherPart->Sockets, TArray<uint32>());
	
	Parts[AddedPartIndex].ConnectedPartIndices.Add(OurParentPartIndex);
	Parts[OurParentPartIndex].ConnectedPartIndices.Add(AddedPartIndex);
	
	for (const int32 Index : OtherPart->ConnectedPartIndices)
	{
		auto* const OtherChildPart = &Other->Parts[Index];
		if (!AttachedParts.Contains(OtherChildPart))
		{
			Attach(Other, OtherChildPart, AttachedParts, AddedPartIndex);
		}
	}

	return AddedPartIndex;
}

void UUHAttachable::SetSimulatePhysics(bool bSimulationEnabled)
{
	Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent())->SetSimulatePhysics(bSimulationEnabled); 
}
