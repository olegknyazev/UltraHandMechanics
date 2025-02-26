#include "UHPlayerController.h"

#include "UHManipulator.h"
#include "UHPicker.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"

AUHPlayerController::AUHPlayerController()
{
	Picker = CreateDefaultSubobject<UUHPicker>(TEXT("Picker"));
}

void AUHPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AUHPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* const EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AUHPlayerController::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AUHPlayerController::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUHPlayerController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUHPlayerController::Look);
		EnhancedInputComponent->BindAction(UltraHandStartAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandStart);
	}
	
	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AUHPlayerController::Jump()
{
	if (!GetCharacter())
	{
		return;
	}

	GetCharacter()->Jump();
}

void AUHPlayerController::StopJumping()
{
	if (!GetCharacter())
	{
		return;
	}

	GetCharacter()->StopJumping();
}

void AUHPlayerController::UltraHandStart()
{
	if (Picker)
	{
		if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
		{
			Manipulator->StartManipulation(Picker->SelectedBlock);
		}
	}
}

void AUHPlayerController::Move(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	const auto MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FRotationMatrix RotationMatrix(YawRotation);
	const FVector ForwardDirection = RotationMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDirection = RotationMatrix.GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}

void AUHPlayerController::Look(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	const auto LookAxisVector = Value.Get<FVector2D>();

	GetPawn()->AddControllerYawInput(LookAxisVector.X);
	GetPawn()->AddControllerPitchInput(LookAxisVector.Y);
}