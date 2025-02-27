#include "UHCharacherCameraController.h"

#include "GameFramework/SpringArmComponent.h"


UUHCharacherCameraController::UUHCharacherCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHCharacherCameraController::ActivateRegularMode()
{
	Mode = EMode::Regular;
	
	if (SpringArm)
	{
		SpringArm->bUsePawnControlRotation = true;
	}
}

void UUHCharacherCameraController::ActivateUltraHandPickingMode()
{
	Mode = EMode::UltraHandPicking;
	
	if (SpringArm)
	{
		SpringArm->bUsePawnControlRotation = true;
	}
}

void UUHCharacherCameraController::ActivateUltraHandManipulatingMode()
{
	Mode = EMode::UltraHandManipulating;
	
	if (SpringArm)
	{
		SpringArm->SetWorldRotation(SpringArm->PreviousDesiredRot);
		SpringArm->bUsePawnControlRotation = false;
	}
}

void UUHCharacherCameraController::BeginPlay()
{
	Super::BeginPlay();

	if (SpringArm)
	{
		const auto& Settings = GetSettings(Mode);
		SpringArm->SocketOffset = Settings.Offset;
		SpringArm->TargetArmLength = Settings.Distance;
	}
}

void UUHCharacherCameraController::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SpringArm)
	{
		const auto& Settings = GetSettings(Mode);
		
		if (Mode == EMode::UltraHandManipulating)
		{
			FRotator DesiredRotation = Cast<APawn>(GetOwner())->GetControlRotation();
			DesiredRotation.Pitch = -20.f;
			FQuat Rotation = FMath::QInterpTo(SpringArm->GetComponentRotation().Quaternion(), DesiredRotation.Quaternion(), DeltaTime, Settings.BlendSpeed);
			
			SpringArm->SetWorldRotation(Rotation);
		}
		
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Settings.Offset, DeltaTime, Settings.BlendSpeed);
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Settings.Distance, DeltaTime, Settings.BlendSpeed);
	}
}

const FUH3rdPersonCameraSettings& UUHCharacherCameraController::GetSettings(EMode Mode) const
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
