#include "UHPlayerController.h"

#include "UHManipulator.h"
#include "UHPicker.h"
#include "UHCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"


void FUHPlayerControllerMode::Enter(AUHPlayerController* Controller)
{
}

void FUHPlayerControllerMode::Leave(AUHPlayerController* Controller)
{
}

EControlMode FUHPlayerControllerMode::GetMode() const
{
	return EControlMode::Regular;
}


// Regular

void FUHPlayerControllerRegularMode::Enter(AUHPlayerController* Controller)
{
}

void FUHPlayerControllerRegularMode::Leave(AUHPlayerController* Controller)
{
}

EControlMode FUHPlayerControllerRegularMode::GetMode() const
{
	return EControlMode::Regular;
}


// Picking

void FUHPlayerControllerPickingMode::Enter(AUHPlayerController* Controller)
{
	if (ensure(Controller))
	{
		Controller->Picker->SetPickingEnabled(true);
	}
}

void FUHPlayerControllerPickingMode::Leave(AUHPlayerController* Controller)
{
	if (ensure(Controller))
	{
		Controller->Picker->SetPickingEnabled(false);
	}
}

EControlMode FUHPlayerControllerPickingMode::GetMode() const
{
	return EControlMode::UltraHandPicking;
}


// Manipulating

void FUHPlayerControllerManipulatingMode::Enter(AUHPlayerController* Controller)
{
	if (!ensure(ManipulatedBlockPart))
	{
		return;
	}

	if (!ensure(Controller))
	{
		return;
	}
	
	FRotator NewRotation = Controller->GetControlRotation();
	NewRotation.Yaw = (ManipulatedBlockPart->GetComponentLocation() - Controller->GetPawn()->GetActorLocation()).Rotation().Yaw;
	Controller->SetControlRotation(NewRotation);

	if (UUHManipulator* Manipulator = Controller->GetPawnManipulator())
	{
		Manipulator->StartManipulation(ManipulatedBlockPart);
	}
}

void FUHPlayerControllerManipulatingMode::Leave(AUHPlayerController* Controller)
{
	if (UUHManipulator* Manipulator = Controller->GetPawnManipulator())
	{
		Manipulator->StopManipulation();
	}

	ManipulatedBlockPart = nullptr;
}

EControlMode FUHPlayerControllerManipulatingMode::GetMode() const
{
	return EControlMode::UltraHandManipulation;
}


// Turning

void FUHPlayerControllerTurningMode::Enter(AUHPlayerController* Controller)
{
}

void FUHPlayerControllerTurningMode::Leave(AUHPlayerController* Controller)
{
}

EControlMode FUHPlayerControllerTurningMode::GetMode() const
{
	return EControlMode::UltraHandTurning;
}


// --

AUHPlayerController::AUHPlayerController()
{
	Picker = CreateDefaultSubobject<UUHPicker>(TEXT("Picker"));
}

void AUHPlayerController::PlayerTick(float DeltaTime)
{
	if (PawnManipulator)
	{
		if (PawnManipulator->IsManipulating())
		{
			TimeSinceLastUltraHandInput += DeltaTime;

			if (RotationCorrectionDelay > 0.f && RotationCorrectionSpeed > 0.f && TimeSinceLastUltraHandInput >= RotationCorrectionDelay)
			{
				const FVector Offset = PawnManipulator->GetOffset();
				const float YawOffset = Offset.HeadingAngle();
				const float YawDelta = FMath::FInterpTo(0.f, FMath::RadiansToDegrees(YawOffset), DeltaTime, RotationCorrectionSpeed);
				const float DistanceScale = FMath::Cos(YawOffset);
				PawnManipulator->MoveRelative(FVector::ForwardVector * Offset.X * (DistanceScale - 1.f));
				GetPawn()->AddControllerYawInput(YawDelta);
			}
		}
	}
	
	Super::PlayerTick(DeltaTime);
}

void AUHPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	EnforceMaxRotationSpeed(DeltaTime);

	EnforceMaxYawOffset();

	RotationInput.Yaw *= DeviationBasedMovementScale(FMath::Sign(RotationInput.Yaw) * UE_PI / 2.f);
}

void AUHPlayerController::EnforceMaxRotationSpeed(const float DeltaTime)
{
	if (MaxRotationSpeed > 0.f)
	{
		const float MaxRotationPerFrame = MaxRotationSpeed * DeltaTime;
		RotationInput.Pitch = FMath::Clamp(RotationInput.Pitch, -MaxRotationPerFrame, MaxRotationPerFrame);
		RotationInput.Yaw = FMath::Clamp(RotationInput.Yaw, -MaxRotationPerFrame, MaxRotationPerFrame);
	}
}

void AUHPlayerController::EnforceMaxYawOffset()
{
	if (MaxYawOffset > 0.f)
	{
		if (auto* const Manipulator = GetPawn()->FindComponentByClass<UUHManipulator>())
		{
			if (Manipulator->IsManipulating() && TimeSinceLastUltraHandInput == 0.f)
			{
				const float YawOffset = FMath::RadiansToDegrees(Manipulator->GetOffset().HeadingAngle());
				const float YawOffsetRatio = FMath::Clamp(FMath::Abs(YawOffset) / MaxYawOffset, 0.f, 1.f);
				RotationInput.Yaw *= 1.f - FMath::Square(YawOffsetRatio);
			}
		}
	}
}

EControlMode AUHPlayerController::GetControlMode() const
{
	return ensure(!ModeStack.IsEmpty()) ? ModeStack.Last()->GetMode() : EControlMode::Regular;
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
		EnhancedInputComponent->BindAction(UltraHandAttachAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandAttach);
		EnhancedInputComponent->BindAction(UltraHandDetachAction, ETriggerEvent::Triggered, this, &AUHPlayerController::UltraHandDetach);
	}

	EnterMode(&RegularMode);
}

void AUHPlayerController::OnPossess(APawn* Pawn)
{
	Super::OnPossess(Pawn);

	PawnManipulator = Pawn->FindComponentByClass<UUHManipulator>();
}

void AUHPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	PawnManipulator = nullptr;
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
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandStart"));

	EnterMode(&PickingMode);
}

void AUHPlayerController::UltraHandPick()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandPick"));
	
	if (UPrimitiveComponent* const SelectedPart = Picker->GetSelectedBlockPart())
	{
		ManipulatingMode.ManipulatedBlockPart = SelectedPart;

		LeaveMode(&PickingMode);
		EnterMode(&ManipulatingMode);
	}
}

void AUHPlayerController::UltraHandStop()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandStop"));

	LeaveModesUpTo(&RegularMode);
}

void AUHPlayerController::Move(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	auto MovementVector = Value.Get<FVector2D>();
	MovementVector *= DeviationBasedMovementScale(FMath::Atan2(MovementVector.X, MovementVector.Y));
	
	const FRotator ControlRotation = GetControlRotation();
	const FRotationMatrix RotationMatrix{FRotator{0, ControlRotation.Yaw, 0}};
	const FVector ForwardDirection = RotationMatrix.GetUnitAxis(EAxis::X);
	const FVector RightDirection = RotationMatrix.GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
	
	TimeSinceLastUltraHandInput = 0.f;
}

void AUHPlayerController::Look(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	const auto LookDelta = Value.Get<FVector2D>();

	AddYawInput(LookDelta.X);
	AddPitchInput(LookDelta.Y);
}

void AUHPlayerController::UltraHandMove(const FInputActionValue& Value)
{
	if (PawnManipulator)
	{
		PawnManipulator->MoveRelative(Value.Get<FVector>());
	
		TimeSinceLastUltraHandInput = 0.f;
	}
}

void AUHPlayerController::UltraHandLook(const FInputActionValue& Value)
{
	if (!GetPawn())
	{
		return;
	}
	
	const auto YawDelta = Value.Get<float>();
	
	AddYawInput(YawDelta);
	
	TimeSinceLastUltraHandInput = 0.f;
}

void AUHPlayerController::UltraHandTurnStart()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnStart"));

	EnterMode(&TurningMode);
}

void AUHPlayerController::UltraHandTurnStop()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnStop"));

	LeaveMode(&TurningMode);
}

void AUHPlayerController::UltraHandTurnLeft()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnLeft"));
	
	if (PawnManipulator)
	{
		PawnManipulator->TurnLeft();
	}
}

void AUHPlayerController::UltraHandTurnRight()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnRight"));
	
	if (PawnManipulator)
	{
		PawnManipulator->TurnRight();
	}
}

void AUHPlayerController::UltraHandTurnUp()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnUp"));
	
	if (PawnManipulator)
	{
		PawnManipulator->TurnUp();
	}
}

void AUHPlayerController::UltraHandTurnDown()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandTurnDown"));
	
	if (PawnManipulator)
	{
		PawnManipulator->TurnDown();
	}
}

void AUHPlayerController::UltraHandAttach()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandAttach"));

	if (PawnManipulator)
	{
		if (PawnManipulator->StartSticking())
		{
			UltraHandStop();
		}
	}
}

void AUHPlayerController::UltraHandDetach()
{
	UE_LOG(LogPlayerController, Display, TEXT("UltraHandDetach"));

	if (PawnManipulator)
	{
		PawnManipulator->Detach();
	}
}

void AUHPlayerController::EnterMode(FUHPlayerControllerMode* Mode)
{
	ModeStack.Push(Mode);

	Mode->Enter(this);
	
	if (UEnhancedInputLocalPlayerSubsystem* const InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->AddMappingContext(Mode->MappingContext, ModeStack.Num());
	}

	SyncCameraWithCurrentMode();
}

void AUHPlayerController::LeaveMode(FUHPlayerControllerMode* Mode)
{
	if (!ensure(ModeStack.Last() == Mode))
	{
		return;
	}
	
	ModeStack.Pop(EAllowShrinking::No);

	Mode->Leave(this);

	if (UEnhancedInputLocalPlayerSubsystem* const InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->RemoveMappingContext(Mode->MappingContext);
	}
	
	SyncCameraWithCurrentMode();
}

void AUHPlayerController::LeaveModesUpTo(FUHPlayerControllerMode* Mode)
{
	while (!ModeStack.IsEmpty() && ModeStack.Last() != Mode)
	{
		FUHPlayerControllerMode* Popped = ModeStack.Pop(EAllowShrinking::No);
		
		Popped->Leave(this);
		
		if (UEnhancedInputLocalPlayerSubsystem* const InputSubsystem = GetInputSubsystem())
		{
			InputSubsystem->RemoveMappingContext(Popped->MappingContext);
		}
	}

	ensure(!ModeStack.IsEmpty());

	SyncCameraWithCurrentMode();
}

void AUHPlayerController::SyncCameraWithCurrentMode()
{
	if (auto* const Character = GetUltraHandCharacter())
	{
		switch (GetControlMode())
		{
		case EControlMode::Regular:
			Character->ActivateRegularCamera();
			break;
		case EControlMode::UltraHandPicking:
			Character->ActivateUltraHandPickingCamera();
			break;
		case EControlMode::UltraHandManipulation:
			Character->ActivateUltraHandManipulatingCamera(ManipulatingMode.ManipulatedBlockPart);
			break;
		case EControlMode::UltraHandTurning:
			Character->ActivateUltraHandManipulatingCamera(ManipulatingMode.ManipulatedBlockPart);
			break;
		}
	}
}

UEnhancedInputLocalPlayerSubsystem* AUHPlayerController::GetInputSubsystem() const
{
	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
}

AUHCharacter* AUHPlayerController::GetUltraHandCharacter() const
{
	return Cast<AUHCharacter>(GetPawn());
}

float AUHPlayerController::DeviationBasedMovementScale(float LocalHeadingAngle) const
{
	if (MaxDistanceOffset > 0.f)
	{
		if (PawnManipulator)
		{
			if (PawnManipulator->IsManipulating())
			{
				const FVector Deviation = PawnManipulator->GetDeviation();
				const float DeviationRatio = FMath::Clamp(Deviation.Length() / MaxDistanceOffset, 0.f, 1.f);
				const float DeviationHeading = Deviation.HeadingAngle();
				const float AlignmentFactor = FMath::Clamp(FMath::Cos(DeviationHeading - LocalHeadingAngle), 0.f, 1.f);
				return 1.f - FMath::Square(DeviationRatio) * AlignmentFactor;
			}
		}
	}

	return 1.f;
}

UUHManipulator* AUHPlayerController::GetPawnManipulator() const
{
	return PawnManipulator;
}
