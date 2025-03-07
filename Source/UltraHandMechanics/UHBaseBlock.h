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

	AUHBaseBlock();

	void SetHighlightedPart(UStaticMeshComponent* Component);
	bool AnyPartHighlighted() const;

	void Reroot(USceneComponent* NewRoot);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
};
