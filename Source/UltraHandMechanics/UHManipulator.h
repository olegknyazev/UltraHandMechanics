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
	UPROPERTY(EditAnywhere)
	float BlockRotationSpeed;

	UPROPERTY(EditAnywhere)
	float SnapDegree;
	
	UUHManipulator();

	void StartManipulation(UUHBlock* Block);
	void StopManipulation();
	
	void MoveRelative(const FVector& Offset);

	void TurnLeft();
	void TurnRight();
	void TurnUp();
	void TurnDown();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTransform GetOriginTransform() const;
	FRotator SnapRotation(const FRotator& Rotation) const;
	FQuat SnapRotation(const FQuat& Rotation) const;
	
	UPROPERTY()
	UUHBlock* BlockBeingManipulated;

	FVector BlockRelativeLocation;
	FQuat BlockRelativeCurrentRotation;
	FQuat BlockRelativeTargetRotation;
};
