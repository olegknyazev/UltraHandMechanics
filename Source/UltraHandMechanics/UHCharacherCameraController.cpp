#include "UHCharacherCameraController.h"

#include "GameFramework/SpringArmComponent.h"


UUHCharacherCameraController::UUHCharacherCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHCharacherCameraController::ActivateRegularMode()
{
	Mode = EMode::Regular;
}

void UUHCharacherCameraController::ActivateUltraHandMode()
{
	Mode = EMode::UltraHand;
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
		SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, Settings.Offset, DeltaTime, BlendSpeed);
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Settings.Distance, DeltaTime, BlendSpeed);
	}
}

const FUH3rdPersonCameraSettings& UUHCharacherCameraController::GetSettings(EMode Mode) const
{
	switch (Mode)
	{
	case EMode::Regular:
		return RegularSettings;
	case EMode::UltraHand:
		return UltraHandSettings;
	default:
		return RegularSettings;
	}
}
