#include "UHPlayerController.h"

#include "UHManipulator.h"
#include "UHPicker.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "UltraHandMechanicsCharacter.h"
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
		EnhancedInputComponent->BindAction(UltraHandPickAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandPick);
		EnhancedInputComponent->BindAction(UltraHandStopAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandStop);
		EnhancedInputComponent->BindAction(UltraHandMoveAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandMove);
		EnhancedInputComponent->BindAction(UltraHandLookAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandLook);
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
	UE_LOG(LogTemp, Display, TEXT("UltraHandStart"));
	
	Picker->SetPickingEnabled(true);
	
	if (auto* const InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(UltraHandPickingMappingContext, 1);
	}

	if (auto* const Character = GetUltraHandCharacter())
	{
		Character->ActivateUltraHandCamera();
	}
}

void AUHPlayerController::UltraHandPick()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandPick"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		if (auto* const SelectedBlock = Picker->SelectedBlock)
		{
			Manipulator->StartManipulation(SelectedBlock);
		}
	}
	
	if (auto* const InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(UltraHandPickingMappingContext);
		InputSubsystem->AddMappingContext(UltraHandManipulatingMappingContext, 2);
	}
}

void AUHPlayerController::UltraHandStop()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandStop"));
	
	Picker->SetPickingEnabled(false);
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->StopManipulation();
	}
	
	if (auto* const InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(UltraHandPickingMappingContext);
		InputSubsystem->RemoveMappingContext(UltraHandManipulatingMappingContext);
	}
	
	if (auto* const Character = GetUltraHandCharacter())
	{
		Character->ActivateRegularCamera();
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

void AUHPlayerController::UltraHandMove(const FInputActionValue& Value)
{
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->MoveRelative(Value.Get<FVector>());
	}
}

void AUHPlayerController::UltraHandLook(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	const auto LookYawValue = Value.Get<float>();
	
	GetPawn()->AddControllerYawInput(LookYawValue);
}

AUltraHandMechanicsCharacter* AUHPlayerController::GetUltraHandCharacter() const
{
	return Cast<AUltraHandMechanicsCharacter>(GetPawn());
}
