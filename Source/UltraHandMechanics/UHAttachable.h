#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHAttachable.generated.h"


class UUHBlockMovementComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogUHAttachable, Log, All);


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


UENUM()
enum class EUHAttachableState
{
	Idle,
	Attaching,
	Sticking
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

	bool IsAttachInProgress() const;
	void StartAttaching(float InMaxAttachDistance);
	void StopAttaching();

	bool IsStickInProgress() const;
	bool StartSticking();

	void Detach(USceneComponent* PartToDetach);

	virtual void BeginPlay() override;
	
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	struct FSocketHandle
	{
		TObjectPtr<UUHAttachable> Attachable;
		int32 PartIndex = -1;
		int32 SocketIndex = -1;

		bool IsSet() const;
		void Set(UUHAttachable* InAttachable, int32 InPartIndex, int32 InSocketIndex);
		void Reset();
		
		UPrimitiveComponent* GetPrimitive() const;
		FVector GetWorldLocation() const;
	};
	
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

	void CopyPartsFrom(
		const UUHAttachable* Other,
		uint32 OtherParentPartIndex,
		uint32 OurParentPartIndex,
		TSet<uint32>& CopiedParts);
	
	UStaticMeshComponent* CopyMeshComponent(const UStaticMeshComponent* Prototype, UPrimitiveComponent* ParentComponent);

	EUHAttachableState State;
	float MaxAttachDistance;

	UPROPERTY(Transient)
	TArray<FUHAttachmentPart> Parts;

	UPROPERTY(Transient)
	UUHAttachable* CurrentTarget;

	FSocketHandle CurrentOurSocketHandle;
	FSocketHandle CurrentTheirSocketHandle;
	
	float CurrentDistance;

	FRotator TheirTargetRotation;
	FRotator OurTargetRotation;

	float RemainingDistance = -1.f;
	float RemainingAngle = -1.f;
};