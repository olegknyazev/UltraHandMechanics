#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHCharacherCameraController.generated.h"


class UUHBlock;
class UCameraComponent;
class USpringArmComponent;

USTRUCT()
struct ULTRAHANDMECHANICS_API FUH3rdPersonCameraSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector Offset;

	UPROPERTY(EditAnywhere)
	float Distance;

	UPROPERTY(EditAnywhere)
	float BlendSpeed;
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUH3rdPersonManipulatingCameraSettings : public FUH3rdPersonCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MinPitch;

	UPROPERTY(EditAnywhere)
	float MaxPitch;

	UPROPERTY(EditAnywhere)
	float MaxDistance;

	UPROPERTY(EditAnywhere)
	float MaxPitchHorizontalDistance;
	
	UPROPERTY(EditAnywhere)
	float MaxPitchVerticalDistance;
	
	UPROPERTY(EditAnywhere)
	float MaxPitchHorizontalContribution;
	
	UPROPERTY(EditAnywhere)
	float MaxPitchVerticalContribution;
	
	UPROPERTY(EditAnywhere)
	float MaxDistanceCombinedDistance;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHCharacherCameraController : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USpringArmComponent* SpringArm;

	UPROPERTY()
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	FUH3rdPersonCameraSettings RegularSettings;

	UPROPERTY(EditAnywhere)
	FUH3rdPersonCameraSettings UltraHandPickingSettings;

	UPROPERTY(EditAnywhere)
	FUH3rdPersonManipulatingCameraSettings UltraHandManipulatingSettings;

	UUHCharacherCameraController();

	void ActivateRegularMode();
	void ActivateUltraHandPickingMode();
	void ActivateUltraHandManipulatingMode(UUHBlock* InBlockBeingManipulated);

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class EMode
	{
		Regular,
		UltraHandPicking,
		UltraHandManipulating
	};

	const FUH3rdPersonCameraSettings& GetSettings(EMode Mode) const;

	EMode Mode = EMode::Regular;

	UPROPERTY()
	UUHBlock* BlockBeingManipulated;

	FRotator GetControlRotation() const;
	FVector GetBlockRelativeLocation() const;
	void ComputeManipulatingCameraFactors(float& DistanceFactor, float& PitchFactor) const;
};
