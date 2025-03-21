#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHPicker.generated.h"


class AUHBaseBlock;
class UUHBlockHighlight;
class UPrimitiveComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogUHPicker, Log, All);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHPicker : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float MaxDistanceToPlayer;
	
	UUHPicker();

	AUHBaseBlock* GetSelectedBlock() const;
	UPrimitiveComponent* GetSelectedBlockPart() const;
	
	void SetPickingEnabled(bool bInEnabled);
	bool IsPickingEnabled() const;
	
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetSelectedPart(UPrimitiveComponent* Part);
	UPrimitiveComponent* TraceMeshUnderAim() const;

	bool bPickingEnabled;

	UPROPERTY(Transient)
	AUHBaseBlock* SelectedBlock;
	
	UPROPERTY(Transient)
	UPrimitiveComponent* SelectedBlockPart;
};
