#include "UHAttachableVisualizer.h"

#include "UltraHandMechanics/UHAttachable.h"

void FUHAttachableVisualizer::DrawVisualization(
	const UActorComponent* Component,
	const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	auto* Attachable = Cast<UUHAttachable>(Component);
	if (!ensure(Attachable))
	{
		return;
	}

	if (!ensure(Attachable->AttachablePrimitive))
	{
		return;
	}

	const FTransform& AttachableTransform = Attachable->AttachablePrimitive->GetComponentTransform();
	for (const auto& Socket : Attachable->Sockets)
	{
		const FVector Position = AttachableTransform.TransformPosition(Socket.Location);
		PDI->DrawPoint(Position, FColor::Red, 10.f, SDPG_World);
	}
}
