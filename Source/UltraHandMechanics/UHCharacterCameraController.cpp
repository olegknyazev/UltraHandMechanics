#include "UHCharacterCameraController.h"

#include "GameFramework/SpringArmComponent.h"


UUHCharacterCameraController::UUHCharacterCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHCharacterCameraController::ActivateRegularMode()
{
	Mode = EMode::Regular;
	ManipulatedComponent = nullptr;
	
	if (SpringArm)
	{
		SpringArm->bUsePawnControlRotation = true;
	}
}

void UUHCharacterCameraController::ActivateUltraHandPickingMode()
{
	Mode = EMode::UltraHandPicking;
	ManipulatedComponent = nullptr;
	
	if (SpringArm)
	{
		SpringArm->bUsePawnControlRotation = true;
	}
}

void UUHCharacterCameraController::ActivateUltraHandManipulatingMode(USceneComponent* InManipulatedComponent)
{
	ensure(InManipulatedComponent);
	
	Mode = EMode::UltraHandManipulating;
	ManipulatedComponent = InManipulatedComponent;
	
	if (SpringArm)
	{
		SpringArm->SetWorldRotation(SpringArm->PreviousDesiredRot);
		SpringArm->bUsePawnControlRotation = false;
	}
}

void UUHCharacterCameraController::BeginPlay()
{
	Super::BeginPlay();

	if (SpringArm)
	{
		const auto& Settings = GetSettings(Mode);
		SpringArm->SocketOffset = Settings.Offset;
		SpringArm->TargetArmLength = Settings.Distance;
	}
}

void UUHCharacterCameraController::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SpringArm)
	{
		if (Mode == EMode::UltraHandManipulating)
		{
			const auto& Settings = UltraHandManipulatingSettings;

			float DistanceFactor = 0.f;
			float PitchFactor = 0.f;
			ComputeManipulatingCameraFactors(DistanceFactor, PitchFactor);
			
			FRotator DesiredRotation = GetControlRotation();
			DesiredRotation.Pitch = FMath::Lerp(Settings.MinPitch, Settings.MaxPitch, PitchFactor);
			FQuat CurrentRotation = FMath::QInterpTo(SpringArm->GetComponentRotation().Quaternion(), DesiredRotation.Quaternion(), DeltaTime, Settings.BlendSpeed);

			float DesiredDistance = FMath::Lerp(Settings.Distance, Settings.MaxDistance, DistanceFactor);
			
			SpringArm->SetWorldRotation(CurrentRotation);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Settings.Offset, DeltaTime, Settings.BlendSpeed);
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredDistance, DeltaTime, Settings.BlendSpeed);
		}
		else
		{
			const auto& Settings = GetSettings(Mode);
			SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Settings.Offset, DeltaTime, Settings.BlendSpeed);
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Settings.Distance, DeltaTime, Settings.BlendSpeed);
		}
	}
}

const FUH3rdPersonCameraSettings& UUHCharacterCameraController::GetSettings(EMode Mode) const
{
	switch (Mode)
	{
	case EMode::Regular:
		return RegularSettings;
		
	case EMode::UltraHandPicking:
		return UltraHandPickingSettings;
		
	case EMode::UltraHandManipulating:
		return UltraHandManipulatingSettings;
		
	default:
		return RegularSettings;
	}
}

FRotator UUHCharacterCameraController::GetControlRotation() const
{
	if (auto* const OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return OwnerPawn->GetControlRotation();
	}
	return FRotator::ZeroRotator;
}

FVector UUHCharacterCameraController::GetBlockRelativeLocation() const
{
	if (!ManipulatedComponent)
	{
		return FVector::Zero();
	}
	
	FRotator Rotation{GetControlRotation()};
	Rotation.Pitch = 0.f;
	FTransform OriginTransform{Rotation, SpringArm->GetComponentLocation()};
	return OriginTransform.InverseTransformPosition(ManipulatedComponent->GetComponentLocation());
}

void UUHCharacterCameraController::ComputeManipulatingCameraFactors(float& DistanceFactor, float& PitchFactor) const
{
	const FUH3rdPersonManipulatingCameraSettings& Settings = UltraHandManipulatingSettings;
	
	FVector BlockOffset = GetBlockRelativeLocation();
	float HorizontalFactor = FMath::Clamp(BlockOffset.X / Settings.MaxPitchHorizontalDistance, 0.f, 1.f) * Settings.MaxPitchHorizontalContribution;
	float VerticalFactor = FMath::Clamp(BlockOffset.Z / Settings.MaxPitchVerticalDistance, 0.f, 1.f) * Settings.MaxPitchVerticalContribution;

	PitchFactor = 1.f - FMath::Clamp(HorizontalFactor + VerticalFactor, 0.f, 1.f);
	DistanceFactor = FMath::Clamp(BlockOffset.Length() / Settings.MaxDistanceCombinedDistance, 0.f, 1.f);
}
