#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHBlockHighlight.generated.h"


class UUHAttachable;
class UUHBlockMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHBlockHighlight : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* HighlightedMaterial;

	UPROPERTY()
	UUHBlockMovementComponent* MovementComponent;

	UPROPERTY()
	UUHAttachable* Attachable;
	
	UUHBlockHighlight();

	UPrimitiveComponent* GetPrimitiveComponent() const;
	void SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent);

	bool IsUnderAim() const;
	void SetUnderAim(bool bInUnderAim);

	bool IsManipulated() const;
	void SetManipulated(bool bInManipulated);

private:
	void UpdateMaterial();
	
	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;
	
	UPROPERTY()
	UMaterialInterface* OriginalMaterial;
	
	uint8 bUnderAim:1;
	uint8 bManipulated:1;
	uint8 bMaterialApplied:1;
};
