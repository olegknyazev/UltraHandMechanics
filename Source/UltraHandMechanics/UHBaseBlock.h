#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UHBaseBlock.generated.h"

class UUHAttachable;
class UUHBlock;
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
	UUHBlock* BlockComponent;

	UPROPERTY(EditAnywhere)
	UPhysicalMaterial* ManipulatedPhysicalMaterial;
	
	AUHBaseBlock();

	bool AnyPartHighlighted() const;
	void SetHighlightedPart(UStaticMeshComponent* Component);

	bool IsManipulated() const;
	void SetManipulated(bool bInManipulated);

	void Reroot(USceneComponent* NewRoot);

	bool StartSticking();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	uint8 bManipulated:1;
};
