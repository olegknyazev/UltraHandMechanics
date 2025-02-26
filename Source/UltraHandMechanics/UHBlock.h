#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHBlock.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHBlock : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* HighlightedMaterial;

	UPROPERTY(BlueprintReadWrite)
	UPrimitiveComponent* HighlightablePrimitive;
	
	UUHBlock();

	bool IsHighlighted() const;
	void SetHighlighted(bool bInHighlighted);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	UMaterialInterface* OriginalMaterial;
	
	bool bHighlighted;
};
