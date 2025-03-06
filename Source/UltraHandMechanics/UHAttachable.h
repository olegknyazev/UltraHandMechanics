#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHAttachable.generated.h"


class UPhysicsConstraintComponent;
class UUHBlockMovementComponent;

USTRUCT()
struct ULTRAHANDMECHANICS_API FUHAttachmentSocket
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector Location;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHAttachable : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UPrimitiveComponent* AttachablePrimitive;

	UPROPERTY()
	UUHBlockMovementComponent* MovementComponent;
	
	UPROPERTY(EditAnywhere)
	TArray<FUHAttachmentSocket> Sockets;

	UPROPERTY(EditAnywhere)
	float RotationSpeed;
	
	UPROPERTY(EditAnywhere)
	float MovementSpeed;

	UUHAttachable();

	void StartAttaching(float InMaxAttachDistance);
	void StopAttaching();

	bool StartSticking();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	static FRotator SnappedRelativeRotation(USceneComponent* Component, USceneComponent* Space);

	FRotator OurTargetRotationInWorldSpace() const;
	FRotator TheirTargetRotationInWorldSpace() const;

	void UpdateCurrentTarget();
	void StopSticking();

	void Attach(UUHAttachable* Other);
	
	bool bAttachInProgress;
	bool bStickInProgress;
	float MaxAttachDistance;

	UPROPERTY()
	UUHAttachable* CurrentTarget;
	int32 CurrentTheirSocketIndex;
	int32 CurrentOurSocketIndex;
	float CurrentDistance;

	FRotator TheirTargetRotation;
	FRotator OurTargetRotation;

	float RemainingDistance = -1.f;
	float RemainingAngle = -1.f;
};
