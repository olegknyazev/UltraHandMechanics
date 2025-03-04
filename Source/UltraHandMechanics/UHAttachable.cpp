#include "UHAttachable.h"


UUHAttachable::UUHAttachable()
{
	PrimaryComponentTick.bCanEverTick = true;
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
}
