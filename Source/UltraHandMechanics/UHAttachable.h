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


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHAttachmentPart
{
	GENERATED_BODY()

	UPROPERTY()
	UPrimitiveComponent* PrimitiveComponent;

	UPROPERTY()
	TArray<FUHAttachmentSocket> Sockets;

	UPROPERTY()
	TArray<uint32> ConnectedPartIndices;
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

	void Detach(USceneComponent* PartToDetach);

	virtual void BeginPlay() override;
	
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
	uint32 Attach(
		UUHAttachable* Other,
		const FUHAttachmentPart* OtherPart,
		TSet<const FUHAttachmentPart*>& AttachedParts,
		uint32 OurParentPartIndex);
	
	void SetSimulatePhysics(bool bSimulationEnabled);

	void CopyPartsFrom(
		const UUHAttachable* Other,
		uint32 OtherParentPartIndex,
		uint32 OurParentPartIndex,
		TSet<uint32>& CopiedParts);
	
	bool bAttachInProgress;
	bool bStickInProgress;
	float MaxAttachDistance;

	UPROPERTY()
	TArray<FUHAttachmentPart> Parts;

	UPROPERTY()
	UUHAttachable* CurrentTarget;

	UPROPERTY()
	UPrimitiveComponent* CurrentOurPrimitive;
	
	UPROPERTY()
	UPrimitiveComponent* CurrentTheirPrimitive;
	
	int32 CurrentTheirPartIndex;
	int32 CurrentOurPartIndex;
	int32 CurrentTheirSocketIndex;
	int32 CurrentOurSocketIndex;
	float CurrentDistance;

	FRotator TheirTargetRotation;
	FRotator OurTargetRotation;

	float RemainingDistance = -1.f;
	float RemainingAngle = -1.f;
};