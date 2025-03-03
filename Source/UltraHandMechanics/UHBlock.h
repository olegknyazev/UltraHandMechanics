#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHBlock.generated.h"


class UUHBlockMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHBlock : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* HighlightedMaterial;

	UPROPERTY(BlueprintReadWrite)
	UPrimitiveComponent* HighlightablePrimitive;

	UPROPERTY(BlueprintReadWrite)
	UUHBlockMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere)
	float TargetApproachTime;
	
	UUHBlock();

	bool IsHighlighted() const;
	void SetHighlighted(bool bInHighlighted);

	UPrimitiveComponent* GetPrimitiveComponent() const;

	FVector GetBlockLocation() const;

	void SetTargetPlacement(const FVector& Location, const FRotator& Rotation);
	void ResetTargetPlacement();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	UMaterialInterface* OriginalMaterial;
	
	bool bHighlighted;
	TOptional<FVector> TargetLocation;
	TOptional<FRotator> TargetRotation;
};
