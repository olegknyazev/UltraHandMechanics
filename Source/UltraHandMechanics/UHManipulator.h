#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "UHManipulator.generated.h"


class UUHBlock;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHManipulator : public USceneComponent
{
	GENERATED_BODY()

public:	
	UUHManipulator();

	void StartManipulation(UUHBlock* Block);
	void StopManipulation();
	void MoveRelative(const FVector& Offset);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTransform GetOriginTransform() const;
	
	UPROPERTY()
	UUHBlock* BlockBeingManipulated;

	FVector BlockRelativeLocation;
	FQuat BlockRelativeRotation;
};
