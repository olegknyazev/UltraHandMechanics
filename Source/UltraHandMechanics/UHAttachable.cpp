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
	
	static TAutoConsoleVariable<bool> DebugDrawAttachment(
		TEXT("uh.DebugDrawAttachment"),
		true,
		TEXT("Defines whether the attachment process debug visualization is enabled."),
		ECVF_Default);
	
	static TAutoConsoleVariable<bool> DebugDrawAttachmentTraces(
		TEXT("uh.DebugDrawAttachmentTraces"),
		false,
		TEXT("Defines whether the traces made in the attachment process are visualized."),
		ECVF_Default);
}


// FSocketHandle

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


// FAttachmentOption

UUHAttachable::FAttachmentOption::FAttachmentOption(
	const FSocketHandle& InOurSocket,
	const FSocketHandle& InTheirSocket)
		: OurSocket(InOurSocket)
		, TheirSocket(InTheirSocket)
{
	UpdateDistance();
}

bool UUHAttachable::FAttachmentOption::IsSet() const
{
	return OurSocket.IsSet() && TheirSocket.IsSet();
}

void UUHAttachable::FAttachmentOption::Reset()
{
	OurSocket.Reset();
	TheirSocket.Reset();
	UpdateDistance();
}

void UUHAttachable::FAttachmentOption::UpdateDistance()
{
	Distance = IsSet()
		? FVector::Distance(OurSocket.GetWorldLocation(), TheirSocket.GetWorldLocation())
		: std::numeric_limits<float>::max();
}

UPrimitiveComponent* UUHAttachable::FAttachmentOption::GetOurPrimitive() const
{
	return OurSocket.GetPrimitive();
}

UPrimitiveComponent* UUHAttachable::FAttachmentOption::GetTheirPrimitive() const
{
	return TheirSocket.GetPrimitive();
}


// UUHAttachable

UUHAttachable::UUHAttachable()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

bool UUHAttachable::IsAttachingInProgress() const
{
	return State == EUHAttachableState::Attaching;
}

void UUHAttachable::StartAttaching(float InMaxAttachDistance)
{
	if (!Parts.IsEmpty())
	{
		State = EUHAttachableState::Attaching;
		MaxAttachDistance = InMaxAttachDistance;
		CurrentAttachmentOption.Reset();
	}
}

void UUHAttachable::StopAttaching()
{
	StopSticking();
	State = EUHAttachableState::Idle;
}

bool UUHAttachable::IsStickingInProgress() const
{
	return State == EUHAttachableState::Sticking;
}

bool UUHAttachable::StartSticking()
{
	if (IsAttachingInProgress() && CurrentAttachmentOption.IsSet() && CurrentAttachmentOption.Distance < MaxAttachDistance && MovementComponent)
	{
		auto* OurBlock = Cast<AUHBaseBlock>(CurrentAttachmentOption.GetOurPrimitive()->GetOwner());
		if (!ensure(OurBlock))
		{
			return false;
		}

		OurBlock->SetManipulated(false);
		OurBlock->Reroot(CurrentAttachmentOption.GetOurPrimitive());

		State = EUHAttachableState::Sticking;
		OurTargetRotation = SnappedRelativeRotation(CurrentAttachmentOption.GetOurPrimitive(), CurrentAttachmentOption.GetTheirPrimitive());
		TheirTargetRotation = SnappedRelativeRotation(CurrentAttachmentOption.GetTheirPrimitive(), CurrentAttachmentOption.GetOurPrimitive());
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

	FIndicesSet VisitedIndices;
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

		VisitedIndices.Reset();
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

	CurrentAttachmentOption.Reset();
}

void UUHAttachable::CopyPartsFrom(
	const UUHAttachable* Other,
	uint32 OtherParentPartIndex,
	uint32 OurParentPartIndex,
	FIndicesSet& CopiedParts)
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

	if (IsAttachingInProgress())
	{
		DeclineCurrentAttachmentOptionIfTooFar();
		UpdateCurrentAttachmentOption();
		
		if (AttachableCVars::DebugDrawAttachment.GetValueOnGameThread())
		{
			if (CurrentAttachmentOption.IsSet())
			{
				DrawDebugLine(
					GetWorld(),
					CurrentAttachmentOption.OurSocket.GetWorldLocation(),
					CurrentAttachmentOption.TheirSocket.GetWorldLocation(),
					FColor::Cyan);
			}
		}
	}

	if (IsStickingInProgress())
	{
		ensure(CurrentAttachmentOption.IsSet());
		
		const FVector OurSocketLocation = CurrentAttachmentOption.OurSocket.GetWorldLocation();
		const FVector TheirSocketLocation = CurrentAttachmentOption.TheirSocket.GetWorldLocation();

		if (AttachableCVars::DebugDrawAttachment.GetValueOnGameThread())
		{
			DrawDebugLine(
				GetWorld(),
				OurSocketLocation,
				TheirSocketLocation,
				FColor::Red);
		}

		const FQuat OurCurrentRotation = CurrentAttachmentOption.GetOurPrimitive()->GetComponentRotation().Quaternion();
		FQuat OurDeltaQuat = (OurTargetRotationInWorldSpace().Quaternion() * OurCurrentRotation.Inverse());
		OurDeltaQuat.EnforceShortestArcWith(FQuat::MakeFromRotationVector(MovementComponent->AngularVelocity));
		const FVector OurDelta = OurDeltaQuat.ToRotationVector();
		const FVector OurVelocity = OurDelta.GetSafeNormal() * (FMath::Sqrt(FMath::Clamp(OurDelta.Size() / 10.f, 0.f, 1.f)) * 0.7f + 0.3f) * RotationSpeed;
		MovementComponent->AngularVelocity = OurVelocity;
		
		const FQuat TheirCurrentRotation = CurrentAttachmentOption.GetTheirPrimitive()->GetComponentRotation().Quaternion();
		FQuat TheirDeltaQuat = (TheirTargetRotationInWorldSpace().Quaternion() * TheirCurrentRotation.Inverse());
		TheirDeltaQuat.EnforceShortestArcWith(FQuat::MakeFromRotationVector(CurrentAttachmentOption.TheirSocket.Attachable->MovementComponent->AngularVelocity));
		const FVector TheirDelta = TheirDeltaQuat.ToRotationVector();
		const FVector TheirVelocity = TheirDelta.GetSafeNormal() * (FMath::Sqrt(FMath::Clamp(TheirDelta.Size() / 100.f, 0.f, 1.f)) * 0.5f + 0.5f) * RotationSpeed;
		CurrentAttachmentOption.TheirSocket.Attachable->MovementComponent->AngularVelocity = TheirVelocity;

		const FVector Delta = TheirSocketLocation - OurSocketLocation;
		const FVector Velocity = Delta.GetSafeNormal() * (FMath::Sqrt(FMath::Clamp(Delta.Size() / 100.f, 0.f, 1.f)) * 0.5f + 0.5f) * MovementSpeed;
		MovementComponent->Velocity = Velocity;
		CurrentAttachmentOption.TheirSocket.Attachable->MovementComponent->Velocity = -Velocity;

		const float NewRemainingDistance = FVector::Distance(TheirSocketLocation, OurSocketLocation);
		const float NewRemainingAngle = OurDelta.Size();

		if (NewRemainingDistance < 1.f && NewRemainingAngle < 0.01f)
		{
			UE_LOG(LogUHAttachable, Display, TEXT("Sticking complete"));

			Attach(CurrentAttachmentOption.TheirSocket.Attachable);
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
	return CurrentAttachmentOption.GetTheirPrimitive()->GetComponentTransform().TransformRotation(OurTargetRotation.Quaternion()).Rotator();
}

FRotator UUHAttachable::TheirTargetRotationInWorldSpace() const
{
	return CurrentAttachmentOption.GetOurPrimitive()->GetComponentTransform().TransformRotation(TheirTargetRotation.Quaternion()).Rotator();
}

void UUHAttachable::DeclineCurrentAttachmentOptionIfTooFar()
{
	if (CurrentAttachmentOption.IsSet())
	{
		CurrentAttachmentOption.UpdateDistance();
		
		if (CurrentAttachmentOption.Distance > MaxAttachDistance)
		{
			CurrentAttachmentOption.Reset();
		}
	}
}

void UUHAttachable::UpdateCurrentAttachmentOption()
{
	TArray<FAttachmentOption> Options = FindAllAttachmentOptions();
	
	Algo::SortBy(Options, [](const FAttachmentOption& Option) { return Option.Distance; });

	FCollisionQueryParams QueryParams;
	QueryParams.bFindInitialOverlaps = false;

	for (const FAttachmentOption& Option : Options)
	{
		if (Option.Distance > MaxAttachDistance)
		{
			break;
		}

		if (Option.Distance >= CurrentAttachmentOption.Distance - TargetSwitchDistanceThreshold)
		{
			break;
		}
		
		const FVector OurSocketLocation = Option.OurSocket.GetWorldLocation();
		const FVector To = Option.TheirSocket.GetWorldLocation();
		const FVector From = OurSocketLocation + (To - OurSocketLocation).GetClampedToMaxSize(0.1f);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, QueryParams);

		const bool bTraceSuccessful = !Hit.bBlockingHit || Hit.Time > 0.99f;

		if (AttachableCVars::DebugDrawAttachmentTraces.GetValueOnGameThread())
		{
			DrawDebugLine(GetWorld(), Hit.TraceStart, Hit.bBlockingHit ? Hit.Location : Hit.TraceEnd, bTraceSuccessful ? FColor::Green : FColor::Red);
		}
		
		if (bTraceSuccessful)
		{
			CurrentAttachmentOption = Option;
			break;
		}
	}
}

TArray<UUHAttachable::FAttachmentOption> UUHAttachable::FindAllAttachmentOptions()
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
		
		if (AttachableCVars::DebugDrawAttachmentTraces.GetValueOnGameThread())
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

	TArray<FAttachmentOption> Options;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (auto* const OtherAttachable = Overlap.GetActor()->FindComponentByClass<UUHAttachable>())
		{
			for (int32 TheirPart = 0; TheirPart < OtherAttachable->Parts.Num(); ++TheirPart)
			{
				for (int32 TheirSocket = 0; TheirSocket < OtherAttachable->Parts[TheirPart].Sockets.Num(); ++TheirSocket)
				{
					for (int32 OurPart = 0; OurPart < Parts.Num(); ++OurPart)
					{
						for (int32 OurSocket = 0; OurSocket < Parts[OurPart].Sockets.Num(); ++OurSocket)
						{
							Options.Emplace(
								FSocketHandle{this, OurPart, OurSocket},
								FSocketHandle{OtherAttachable, TheirPart, TheirSocket});
						}
					}
				}
			}
		}
	}

	return MoveTemp(Options);
}

void UUHAttachable::StopSticking()
{
	if (IsStickingInProgress())
	{
		MovementComponent->StopMovementImmediately();
		CurrentAttachmentOption.TheirSocket.Attachable->MovementComponent->StopMovementImmediately();

		CurrentAttachmentOption.Reset();
		TheirTargetRotation = FRotator::ZeroRotator;
		OurTargetRotation = FRotator::ZeroRotator;
		
		State = EUHAttachableState::Attaching;
	}
}

void UUHAttachable::Attach(UUHAttachable* Other)
{
	if (!ensure(CurrentAttachmentOption.IsSet()))
	{
		return;
	}
	
	if (!ensure(CurrentAttachmentOption.GetOurPrimitive() == GetOwner()->GetRootComponent()))
	{
		return;
	}

	const FUHAttachmentPart* OtherStartPart = Other->Parts.FindByPredicate([this](const FUHAttachmentPart& P)
	{
		return P.PrimitiveComponent == CurrentAttachmentOption.GetTheirPrimitive();
	});

	if (!ensure(OtherStartPart))
	{
		return;
	}

	TSet<const FUHAttachmentPart*> AttachedParts;
	Attach(Other, OtherStartPart, AttachedParts, CurrentAttachmentOption.OurSocket.PartIndex);

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
