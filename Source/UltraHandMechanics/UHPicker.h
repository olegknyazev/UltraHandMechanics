#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHPicker.generated.h"


class AUHBaseBlock;
class UUHBlockHighlight;


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
	UStaticMeshComponent* GetSelectedPart() const;
	
	void SetPickingEnabled(bool bInEnabled);
	bool IsPickingEnabled() const;
	
	virtual void BeginPlay() override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetSelectedPart(UStaticMeshComponent* Part);
	UStaticMeshComponent* TraceMeshUnderAim() const;

	bool bPickingEnabled;

	UPROPERTY()
	AUHBaseBlock* SelectedBlock;
	
	UPROPERTY()
	UStaticMeshComponent* SelectedMesh;
};
