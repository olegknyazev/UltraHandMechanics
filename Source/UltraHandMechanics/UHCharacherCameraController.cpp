#include "UHCharacherCameraController.h"

#include "GameFramework/SpringArmComponent.h"


FUH3rdPersonCameraSettings FUH3rdPersonCameraSettings::BlendEaseOut(const FUH3rdPersonCameraSettings& Other, float Alpha, float Exp) const
{
	return FUH3rdPersonCameraSettings{
		FMath::InterpEaseOut(Offset, Other.Offset, Alpha, Exp),
		FMath::InterpEaseOut(Distance, Other.Distance, Alpha, Exp)
	};
}


UUHCharacherCameraController::UUHCharacherCameraController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUHCharacherCameraController::ActivateRegularMode()
{
	NewMode = EMode::Regular;
	CurrentBlendTime = BlendTime - CurrentBlendTime;
}

void UUHCharacherCameraController::ActivateUltraHandMode()
{
	NewMode = EMode::UltraHand;
	CurrentBlendTime = BlendTime - CurrentBlendTime;
}

void UUHCharacherCameraController::BeginPlay()
{
	Super::BeginPlay();

	CurrentBlendTime = BlendTime;
	ApplySettings(GetSettings(Mode));
}

void UUHCharacherCameraController::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (NewMode != Mode)
	{
		CurrentBlendTime = FMath::Clamp(CurrentBlendTime + DeltaTime, 0.f, BlendTime);
		
		const auto& CurrentSettings = GetSettings(Mode);
		const auto& NewSettings = GetSettings(NewMode);
		ApplySettings(CurrentSettings.BlendEaseOut(NewSettings, CurrentBlendTime / BlendTime, BlendExponent));
		
		if (CurrentBlendTime >= BlendTime)
		{
			Mode = NewMode;
		}
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

void UUHCharacherCameraController::ApplySettings(const FUH3rdPersonCameraSettings& Settings)
{
	if (SpringArm)
	{
		SpringArm->SocketOffset = Settings.Offset;
		SpringArm->TargetArmLength = Settings.Distance;
	}
}
