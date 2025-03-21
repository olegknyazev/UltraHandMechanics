#include "UHCharacter.h"

#include "UHCharacterCameraController.h"
#include "UHManipulator.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

AUHCharacter::AUHCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;	
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetUsingAbsoluteRotation(true);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Manipulator = CreateDefaultSubobject<UUHManipulator>(TEXT("Manipulator"));
	Manipulator->SetupAttachment(GetRootComponent());

	CameraController = CreateDefaultSubobject<UUHCharacterCameraController>(TEXT("CameraController"));
	CameraController->Camera = FollowCamera;
	CameraController->SpringArm = CameraBoom;
}

void AUHCharacter::ActivateRegularCamera()
{
	CameraController->ActivateRegularMode();
}

void AUHCharacter::ActivateUltraHandPickingCamera()
{
	CameraController->ActivateUltraHandPickingMode();
}

void AUHCharacter::ActivateUltraHandManipulatingCamera(USceneComponent* ManipulatedComponent)
{
	CameraController->ActivateUltraHandManipulatingMode(ManipulatedComponent);
}

void AUHCharacter::Tick(float DeltaSeconds)
{
	GetCharacterMovement()->MaxWalkSpeed = Manipulator->IsManipulating() ? WalkSpeedManipulating : WalkSpeed;
	
	Super::Tick(DeltaSeconds);
}
