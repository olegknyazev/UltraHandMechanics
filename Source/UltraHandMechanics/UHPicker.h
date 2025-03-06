#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHPicker.generated.h"


class AUHBaseBlock;
class UUHBlock;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHPicker : public UActorComponent
{
	GENERATED_BODY()

public:
	UUHPicker();

	AUHBaseBlock* GetSelectedBlock() const;
	UStaticMeshComponent* GetSelectedPart() const;
	
	void SetPickingEnabled(bool bInEnabled);
	bool IsPickingEnabled() const;
	
protected:
	virtual void BeginPlay() override;

public:
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
