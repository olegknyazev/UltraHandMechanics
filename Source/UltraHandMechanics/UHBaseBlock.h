#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UHBaseBlock.generated.h"

class UUHAttachable;
class UUHBlockHighlight;
class UUHBlockMovementComponent;

UCLASS()
class ULTRAHANDMECHANICS_API AUHBaseBlock : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere)
	UUHBlockMovementComponent* MovementComponent;
	
	UPROPERTY(VisibleAnywhere)
	UUHAttachable* AttachableComponent;
	
	UPROPERTY(VisibleAnywhere)
	UUHBlockHighlight* HighlightComponent;

	UPROPERTY(EditAnywhere)
	UPhysicalMaterial* ManipulatedPhysicalMaterial;
	
	AUHBaseBlock();

	bool AnyPartHighlighted() const;
	void SetHighlightedPart(UStaticMeshComponent* Component);

	bool IsManipulated() const;
	void SetManipulated(bool bInManipulated);

	void StartAttaching(float InMaxAttachDistance);
	void StopAttaching();
	void Detach(USceneComponent* PartToDetach);
	
	void Reroot(USceneComponent* NewRoot);

	bool StartSticking();

private:
	uint8 bManipulated:1;
};
