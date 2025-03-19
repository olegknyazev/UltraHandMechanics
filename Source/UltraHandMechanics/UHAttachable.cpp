#include "UHAttachable.h"

#include "UHBaseBlock.h"
#include "UHBlockMovementComponent.h"
#include "UHDebugDrawFunctions.h"
#include "Engine/OverlapResult.h"


DEFINE_LOG_CATEGORY(LogUHAttachable);

namespace AttachableCVars
{
	static TAutoConsoleVariable<bool> DebugDrawSockets(
		TEXT("uh.DebugDrawSockets"),
		false,
		TEXT("Defines whether the sockets debug visualization is enabled."),
		ECVF_Default);
	
	static TAutoConsoleVariable<bool> DebugDrawSticking(
		TEXT("uh.DebugDrawSticking"),
		true,
		TEXT("Defines whether the sticking process debug visualization is enabled."),
		ECVF_Default);
}


bool UUHAttachable::FSocketHandle::IsSet() const
{
	return static_cast<bool>(Attachable);
}

UPrimitiveComponent* UUHAttachable::FSocketHandle::GetPrimitive() const
{
	return ensure(Attachable) ? Attachable->Parts[PartIndex].PrimitiveComponent : nullptr;
}

FVector UUHAttachable::FSocketHandle::GetWorldLocation() const
{
	if (!ensure(Attachable))
	{
		return FVector::Zero();
	}
	const FUHAttachmentPart& Part = Attachable->Parts[PartIndex];
	const FVector SocketLocation = Part.Sockets[SocketIndex].Location;
	return Part.PrimitiveComponent->GetComponentTransform().TransformPosition(SocketLocation);
}

void UUHAttachable::FSocketHandle::Set(UUHAttachable* InAttachable, int32 InPartIndex, int32 InSocketIndex)
{
	Attachable = InAttachable;
	PartIndex = InPartIndex;
	SocketIndex = InSocketIndex;
}

void UUHAttachable::FSocketHandle::Reset()
{
	Attachable = nullptr;
	SocketIndex = -1;
	PartIndex = -1;
}


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
		ensure(CurrentTheirSocketHandle.IsSet());
		ensure(CurrentOurSocketHandle.IsSet());

		auto* OurBlock = Cast<AUHBaseBlock>(CurrentOurSocketHandle.GetPrimitive()->GetOwner());
		if (!ensure(OurBlock))
		{
			return false;
		}

		OurBlock->SetManipulated(false);
		OurBlock->Reroot(CurrentOurSocketHandle.GetPrimitive());
		
		State = EUHAttachableState::Sticking;
		OurTargetRotation = SnappedRelativeRotation(CurrentOurSocketHandle.GetPrimitive(), CurrentTheirSocketHandle.GetPrimitive());
		TheirTargetRotation = SnappedRelativeRotation(CurrentTheirSocketHandle.GetPrimitive(), CurrentOurSocketHandle.GetPrimitive());
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
			UStaticMeshComponent* NewMeshComponent = CopyMeshComponent(OtherMeshComponent, OurParentPart.PrimitiveComponent);

			const uint32 AddedIndex = Parts.Emplace(NewMeshComponent, OtherPart.Sockets);
			Parts[AddedIndex].ConnectedPartIndices.Add(OurParentPartIndex);
			Parts[OurParentPartIndex].ConnectedPartIndices.Add(AddedIndex);
			
			CopyPartsFrom(Other, OtherIndex, AddedIndex, CopiedParts);
		}
	}
}

UStaticMeshComponent* UUHAttachable::CopyMeshComponent(const UStaticMeshComponent* Prototype, UPrimitiveComponent* ParentComponent)
{
	auto* NewMeshComponent = NewObject<UStaticMeshComponent>(GetOwner());
	NewMeshComponent->SetupAttachment(ParentComponent);
	NewMeshComponent->SetWorldLocation(Prototype->GetComponentLocation());
	NewMeshComponent->SetWorldRotation(Prototype->GetComponentRotation());
	NewMeshComponent->SetStaticMesh(Prototype->GetStaticMesh());
	NewMeshComponent->SetMaterial(0, Prototype->GetMaterial(0));
	NewMeshComponent->RegisterComponent();
	NewMeshComponent->WeldTo(ParentComponent);
	return NewMeshComponent;
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

	if (AttachableCVars::DebugDrawSockets.GetValueOnGameThread())
	{
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
	}

	if (IsAttachInProgress())
	{
		UpdateCurrentTarget();

		if (AttachableCVars::DebugDrawSticking.GetValueOnGameThread())
		{
			if (CurrentTarget != nullptr)
			{
				ensure(CurrentOurSocketHandle.IsSet());
				ensure(CurrentTheirSocketHandle.IsSet());
				
				DrawDebugLine(
					GetWorld(),
					CurrentOurSocketHandle.GetWorldLocation(),
					CurrentTheirSocketHandle.GetWorldLocation(),
					FColor::Cyan);
			}
		}
	}

	if (IsStickInProgress())
	{
		ensure(CurrentOurSocketHandle.IsSet());
		ensure(CurrentTheirSocketHandle.IsSet());
		
		const FVector OurSocketLocation = CurrentOurSocketHandle.GetWorldLocation();
		const FVector TheirSocketLocation = CurrentTheirSocketHandle.GetWorldLocation();

		if (AttachableCVars::DebugDrawSticking.GetValueOnGameThread())
		{
			DrawDebugLine(
				GetWorld(),
				OurSocketLocation,
				TheirSocketLocation,
				FColor::Red);
		}

		const FVector OurDelta = (OurTargetRotationInWorldSpace() - CurrentOurSocketHandle.GetPrimitive()->GetComponentRotation()).GetNormalized().Quaternion().ToRotationVector();
		MovementComponent->AngularVelocity = OurDelta * RotationSpeed;
		
		const FVector TheirDelta = (TheirTargetRotationInWorldSpace() - CurrentTheirSocketHandle.GetPrimitive()->GetComponentRotation()).GetNormalized().Quaternion().ToRotationVector();
		CurrentTarget->MovementComponent->AngularVelocity = TheirDelta * RotationSpeed;

		const FVector Delta = TheirSocketLocation - OurSocketLocation;
		const FVector Velocity = Delta.GetSafeNormal() * (FMath::Sqrt(FMath::Clamp(Delta.Size() / 100.f, 0.f, 1.f)) * 0.5f + 0.5f) * MovementSpeed;
		MovementComponent->Velocity = Velocity;
		CurrentTarget->MovementComponent->Velocity = -Velocity;

		const float NewRemainingDistance = FVector::Distance(TheirSocketLocation, OurSocketLocation);
		const float NewRemainingAngle = OurDelta.Size();

		if (NewRemainingDistance < 1.f && NewRemainingAngle < 0.01f)
		{
			UE_LOG(LogUHAttachable, Display, TEXT("Sticking complete"));

			Attach(CurrentTarget);
			StopAttaching();
		}
		else if ((RemainingDistance > 0.f && FMath::Abs(NewRemainingDistance - RemainingDistance) < UE_KINDA_SMALL_NUMBER) &&
			(RemainingAngle > 0.f && FMath::Abs(NewRemainingAngle - RemainingAngle) < UE_KINDA_SMALL_NUMBER))
		{
			UE_LOG(LogUHAttachable, Display, TEXT("Stuck!"));
			
			StopAttaching();
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
	return CurrentTheirSocketHandle.GetPrimitive()->GetComponentTransform().TransformRotation(OurTargetRotation.Quaternion()).Rotator();
}

FRotator UUHAttachable::TheirTargetRotationInWorldSpace() const
{
	return CurrentOurSocketHandle.GetPrimitive()->GetComponentTransform().TransformRotation(TheirTargetRotation.Quaternion()).Rotator();
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
		
		if (AttachableCVars::DebugDrawSticking.GetValueOnGameThread())
		{
			DrawDebugCollisionShape(GetWorld(), InflatedShape, ShapeLocation, FQuat::Identity, FColor::Orange);
		}
		
		OverlapsLocal.Reset();
		GetWorld()->OverlapMultiByChannel(
			OverlapsLocal,
			ShapeLocation,
			FQuat::Identity,
			ECC_Visibility,
			InflatedShape,
			QueryParams);
		Overlaps.Append(OverlapsLocal);
	}

	CurrentOurSocketHandle.Reset();
	CurrentTheirSocketHandle.Reset();
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
								CurrentOurSocketHandle.Set(this, OPI, OSI);
								CurrentTheirSocketHandle.Set(OtherAttachable, TPI, TSI);
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

		CurrentOurSocketHandle.Reset();
		CurrentTheirSocketHandle.Reset();
		CurrentTarget = nullptr;
		CurrentDistance = 0.f;
		TheirTargetRotation = FRotator::ZeroRotator;
		OurTargetRotation = FRotator::ZeroRotator;
		
		State = EUHAttachableState::Attaching;
	}
}

void UUHAttachable::Attach(UUHAttachable* Other)
{
	if (!ensure(CurrentOurSocketHandle.IsSet()))
	{
		return;
	}
	
	if (!ensure(CurrentOurSocketHandle.GetPrimitive() == GetOwner()->GetRootComponent()))
	{
		return;
	}

	const FUHAttachmentPart* OtherStartPart = Other->Parts.FindByPredicate([this](const FUHAttachmentPart& P)
	{
		return P.PrimitiveComponent == CurrentTheirSocketHandle.GetPrimitive();
	});

	if (!ensure(OtherStartPart))
	{
		return;
	}

	TSet<const FUHAttachmentPart*> AttachedParts;
	Attach(Other, OtherStartPart, AttachedParts, CurrentOurSocketHandle.PartIndex);

	MovementComponent->UpdatedComponentShapeMightChange();
	
	Other->GetOwner()->Destroy();
}

uint32 UUHAttachable::Attach(
	UUHAttachable* Other,
	const FUHAttachmentPart* OtherPart,
	TSet<const FUHAttachmentPart*>& AttachedParts,
	uint32 OurParentPartIndex)
{
	UPrimitiveComponent* AttachParent = Parts[OurParentPartIndex].PrimitiveComponent;
	auto* OtherMeshComponent = Cast<UStaticMeshComponent>(OtherPart->PrimitiveComponent);
	
	UStaticMeshComponent* NewMeshComponent = CopyMeshComponent(OtherMeshComponent, AttachParent);

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
