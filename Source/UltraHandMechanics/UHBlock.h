#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHBlock.generated.h"


class UUHAttachable;
class UUHBlockMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHBlock : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* HighlightedMaterial;

	UPROPERTY(EditAnywhere)
	UPhysicalMaterial* ManipulatedPhysicalMaterial;
	
	UPROPERTY()
	UUHBlockMovementComponent* MovementComponent;

	UPROPERTY()
	UUHAttachable* Attachable;
	
	UUHBlock();

	UPrimitiveComponent* GetPrimitiveComponent() const;
	void SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent);

	bool IsHighlighted() const;
	void SetHighlighted(bool bInHighlighted);

	bool IsManipulated() const;
	void SetManipulated(bool bInManipulated);

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateMaterial();
	
	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;
	
	UPROPERTY()
	UMaterialInterface* OriginalMaterial;
	
	uint8 bHighlighted:1;
	uint8 bManipulated:1;
	uint8 bSelectedMaterialApplied:1;
};
