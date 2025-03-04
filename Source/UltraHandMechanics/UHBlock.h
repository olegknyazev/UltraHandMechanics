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

	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* HighlightablePrimitive;

	UPROPERTY(VisibleAnywhere)
	UUHBlockMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere)
	float TargetApproachTime;
	
	UUHBlock();

	bool IsHighlighted() const;
	void SetHighlighted(bool bInHighlighted);

	bool IsManipulated() const;
	void SetManipulated(bool bInManipulated);

	FVector GetBlockLocation() const;
	FRotator GetBlockRotation() const;

	void SetTargetPlacement(const FVector& Location, const FRotator& Rotation);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPrimitiveComponent* GetPrimitiveComponent() const;

	void UpdateMaterial();
	
	UPROPERTY()
	UMaterialInterface* OriginalMaterial;
	
	uint8 bHighlighted:1;
	uint8 bManipulated:1;
	uint8 bSelectedMaterialApplied:1;
	
	FVector TargetLocation;
	FRotator TargetRotation;
};
