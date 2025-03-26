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

	// It's bidirectional: if part A is connected to B, then A.ConnectedPartIndices contains
	// B and B.ConnectedPartIndices contains A.
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

	UPROPERTY(EditAnywhere)
	float TargetSwitchDistanceThreshold;
	
	UUHAttachable();

	bool IsAttachingInProgress() const;
	void StartAttaching(float InMaxAttachDistance);
	void StopAttaching();

	bool IsStickingInProgress() const;
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

	struct FAttachmentOption
	{
		FSocketHandle OurSocket;
		FSocketHandle TheirSocket;
		float Distance = std::numeric_limits<float>::max();

		FAttachmentOption() = default;
		FAttachmentOption(const FSocketHandle& InOurSocket, const FSocketHandle& InTheirSocket);

		bool IsSet() const;
		void Reset();
		void UpdateDistance();
		
		UPrimitiveComponent* GetOurPrimitive() const;
		UPrimitiveComponent* GetTheirPrimitive() const;
	};

	using FIndicesSet = TSet<uint32, DefaultKeyFuncs<uint32>, TSetAllocator<TInlineSparseArrayAllocator<64>>>;
	
	static FRotator SnappedRelativeRotation(USceneComponent* Component, USceneComponent* Space);

	FRotator OurTargetRotationInWorldSpace() const;
	FRotator TheirTargetRotationInWorldSpace() const;

	void DeclineCurrentAttachmentOptionIfTooFar();
	void UpdateCurrentAttachmentOption();
	TArray<FAttachmentOption> FindAllAttachmentOptions();
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
		FIndicesSet& CopiedParts);
	
	UStaticMeshComponent* CopyMeshComponent(const UStaticMeshComponent* Prototype, UPrimitiveComponent* ParentComponent);

	EUHAttachableState State;
	float MaxAttachDistance;

	UPROPERTY(Transient)
	TArray<FUHAttachmentPart> Parts;

	FAttachmentOption CurrentAttachmentOption;

	FRotator TheirTargetRotation;
	FRotator OurTargetRotation;

	float RemainingDistance = -1.f;
	float RemainingAngle = -1.f;
};