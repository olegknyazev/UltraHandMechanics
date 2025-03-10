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
	
	UPROPERTY(EditAnywhere)
	float MaxAttachDistance;

	UPROPERTY(EditAnywhere)
	float MaxHorizontalOffset;

	UPROPERTY(EditAnywhere)
	float MaxVerticalOffset;
	
	UUHManipulator();

	void StartManipulation(UStaticMeshComponent* InManipulatedPart);
	void StopManipulation();
	
	void MoveRelative(const FVector& Offset);

	void TurnLeft();
	void TurnRight();
	void TurnUp();
	void TurnDown();

	bool StartSticking();

	void Detach();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTransform GetOriginTransform() const;
	FRotator SnapRotation(const FRotator& Rotation) const;
	FQuat SnapRotation(const FQuat& Rotation) const;

	FVector ClampOffset(const FVector& Offset) const;

	UPROPERTY()
	UUHBlock* ManipulatedBlock;

	UPROPERTY()
	UStaticMeshComponent* ManipulatedPart;
	
	FVector BlockRelativeLocation;
	FQuat BlockRelativeCurrentRotation;
	FQuat BlockRelativeTargetRotation;
};
