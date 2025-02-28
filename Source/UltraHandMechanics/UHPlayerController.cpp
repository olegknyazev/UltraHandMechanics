#include "UHPlayerController.h"

#include "UHManipulator.h"
#include "UHPicker.h"
#include "UHBlock.h"
#include "UltraHandMechanicsCharacter.h"

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
		EnhancedInputComponent->BindAction(UltraHandPickAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandPick);
		EnhancedInputComponent->BindAction(UltraHandStopAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandStop);
		EnhancedInputComponent->BindAction(UltraHandMoveAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandMove);
		EnhancedInputComponent->BindAction(UltraHandLookAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandLook);
		EnhancedInputComponent->BindAction(UltraHandTurnStartAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnStart);
		EnhancedInputComponent->BindAction(UltraHandTurnStopAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnStop);
		EnhancedInputComponent->BindAction(UltraHandTurnLeftAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnLeft);
		EnhancedInputComponent->BindAction(UltraHandTurnRightAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnRight);
		EnhancedInputComponent->BindAction(UltraHandTurnUpAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnUp);
		EnhancedInputComponent->BindAction(UltraHandTurnDownAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandTurnDown);
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
		Character->ActivateUltraHandPickingCamera();
	}
}

void AUHPlayerController::UltraHandPick()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandPick"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		if (auto* const SelectedBlock = Picker->SelectedBlock)
		{
			FRotator NewRotation = GetControlRotation();
			NewRotation.Yaw = (SelectedBlock->GetBlockLocation() - GetPawn()->GetActorLocation()).Rotation().Yaw;
			SetControlRotation(NewRotation);
			
			Manipulator->StartManipulation(SelectedBlock);

			if (auto* const Character = GetUltraHandCharacter())
			{
				Character->ActivateUltraHandManipulatingCamera(SelectedBlock);
			}
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
		InputSubsystem->RemoveMappingContext(UltraHandManipulatingTurningMappingContext);
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

void AUHPlayerController::UltraHandTurnStart()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnStart"));
	
	if (auto* const InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(UltraHandManipulatingTurningMappingContext, 3);
	}
}

void AUHPlayerController::UltraHandTurnStop()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnStop"));
	
	if (auto* const InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(UltraHandManipulatingTurningMappingContext);
	}
}

void AUHPlayerController::UltraHandTurnLeft()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnLeft"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->TurnLeft();
	}
}

void AUHPlayerController::UltraHandTurnRight()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnRight"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->TurnRight();
	}
}

void AUHPlayerController::UltraHandTurnUp()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnUp"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->TurnUp();
	}
}

void AUHPlayerController::UltraHandTurnDown()
{
	UE_LOG(LogTemp, Display, TEXT("UltraHandTurnDown"));
	
	if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
	{
		Manipulator->TurnDown();
	}
}

AUltraHandMechanicsCharacter* AUHPlayerController::GetUltraHandCharacter() const
{
	return Cast<AUltraHandMechanicsCharacter>(GetPawn());
}
