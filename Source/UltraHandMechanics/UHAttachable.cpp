#include "UHAttachable.h"

#include "Engine/OverlapResult.h"


namespace AttachableLocal
{
	struct FPotentialConnection
	{
		UUHAttachable* Attachable = nullptr;
		FVector TheirSocketLocation;
		FVector OurSocketLocation;
		float Distance = 0.f;

		FPotentialConnection(UUHAttachable* InAttachable, const FVector& InTheirSocketLocation, const FVector& InOurSocketLocation)
			: Attachable(InAttachable)
			, TheirSocketLocation(InTheirSocketLocation)
			, OurSocketLocation(InOurSocketLocation)
			, Distance(FVector::Distance(InTheirSocketLocation, InOurSocketLocation))
		{}
	};
}


UUHAttachable::UUHAttachable()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHAttachable::StartAttaching(float InMaxAttachDistance)
{
	bAttachInProgress = true;
	MaxAttachDistance = InMaxAttachDistance;
}

void UUHAttachable::StopAttaching()
{
	bAttachInProgress = false;
}

void UUHAttachable::BeginPlay()
{
	Super::BeginPlay();
}

void UUHAttachable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	using namespace AttachableLocal;
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const FUHAttachmentSocket& Socket : Sockets)
	{
		const FVector Location = GetOwner()->GetActorTransform().TransformPosition(Socket.Location);
		const FRotator Rotation = GetOwner()->GetActorRotation();
		DrawDebugBox(GetWorld(), Location, FVector(3.f), Rotation.Quaternion(), FColor::Red);
	}

	if (bAttachInProgress && AttachablePrimitive && !Sockets.IsEmpty())
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
		
		TArray<FPotentialConnection, TInlineAllocator<64>> Connections;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (auto* const OtherAttachable = Overlap.GetActor()->FindComponentByClass<UUHAttachable>())
			{
				for (const FUHAttachmentSocket& TheirSocket : OtherAttachable->Sockets)
				{
					const FVector TheirSocketLocation = OtherAttachable->AttachablePrimitive->GetComponentTransform().TransformPosition(TheirSocket.Location);
					
					for (const FUHAttachmentSocket& OurSocket : Sockets)
					{
						Connections.Emplace(
							OtherAttachable,
							TheirSocketLocation,
							AttachableTransform.TransformPosition(OurSocket.Location));
					}
				}
			}
		}

		const auto SelectDistance = [](const FPotentialConnection& X) { return X.Distance; };
		if (const FPotentialConnection* const ShortestConnection = Algo::MinElementBy(Connections, SelectDistance))
		{
			DrawDebugLine(
				GetWorld(),
				ShortestConnection->OurSocketLocation,
				ShortestConnection->TheirSocketLocation,
				FColor::Cyan);
		}
	}
}
