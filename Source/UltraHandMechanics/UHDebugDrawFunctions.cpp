#include "UHDebugDrawFunctions.h"

void DrawDebugCollisionShape(
	const UWorld* World,
	const FCollisionShape& Shape,
	const FVector& Location,
	const FQuat& Rotation,
	const FColor& Color)
{
	static float PointSize = 3.f;
	static int32 SphereSegments = 16;
	
	if (!ensure(World))
	{
		return;
	}

	switch (Shape.ShapeType)
	{
	case ECollisionShape::Line:
		DrawDebugPoint(World, Location, PointSize, Color);
		break;
	case ECollisionShape::Box:
		DrawDebugBox(World, Location, Shape.GetBox(), Rotation, Color);
		break;
	case ECollisionShape::Sphere:
		DrawDebugSphere(World, Location, Shape.GetSphereRadius(), SphereSegments, Color);
		break;
	case ECollisionShape::Capsule:
		DrawDebugCapsule(World, Location, Shape.GetCapsuleHalfHeight(), Shape.GetCapsuleRadius(), Rotation, Color);
		break;
	}
}
