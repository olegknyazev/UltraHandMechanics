#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "UHManipulator.generated.h"


class AUHBaseBlock;
class UUHBlockHighlight;

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
	
	UPROPERTY(EditAnywhere)
	float TargetApproachTime;
	
	UUHManipulator();

	bool IsManipulating() const;
	void StartManipulation(UStaticMeshComponent* InManipulatedPart);
	void StopManipulation();
	
	void MoveRelative(const FVector& Offset);

	void TurnLeft();
	void TurnRight();
	void TurnUp();
	void TurnDown();

	bool StartSticking();

	void Detach();

	FVector GetOffset() const;
	FVector GetError() const;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTransform GetOriginTransform() const;
	FRotator SnapRotation(const FRotator& Rotation) const;
	FQuat SnapRotation(const FQuat& Rotation) const;

	FVector ClampOffset(const FVector& Offset) const;

	UPROPERTY()
	AUHBaseBlock* ManipulatedBlock;

	UPROPERTY()
	UStaticMeshComponent* ManipulatedPart;
	
	FVector BlockRelativeLocation;
	FQuat BlockRelativeCurrentRotation;
	FQuat BlockRelativeTargetRotation;
};
