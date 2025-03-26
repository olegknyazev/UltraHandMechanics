#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UHPlayerController.generated.h"

class AUHPlayerController;
class AUHCharacter;
class UUHPicker;
class UUHManipulator;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;


UENUM(BlueprintType)
enum class EControlMode : uint8
{
	Regular,
	UltraHandPicking,
	UltraHandManipulation,
	UltraHandTurning
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHPlayerControllerMode
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UInputMappingContext* MappingContext;

	virtual ~FUHPlayerControllerMode() = default;

	virtual void Enter(AUHPlayerController* Controller);
	virtual void Leave(AUHPlayerController* Controller);
	virtual EControlMode GetMode() const;
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHPlayerControllerRegularMode : public FUHPlayerControllerMode
{
	GENERATED_BODY()
	
	virtual void Enter(AUHPlayerController* Controller) override;
	virtual void Leave(AUHPlayerController* Controller) override;
	virtual EControlMode GetMode() const override;
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHPlayerControllerPickingMode : public FUHPlayerControllerMode
{
	GENERATED_BODY()
	
	virtual void Enter(AUHPlayerController* Controller) override;
	virtual void Leave(AUHPlayerController* Controller) override;
	virtual EControlMode GetMode() const override;
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHPlayerControllerManipulatingMode : public FUHPlayerControllerMode
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	UPrimitiveComponent* ManipulatedBlockPart;
	
	virtual void Enter(AUHPlayerController* Controller) override;
	virtual void Leave(AUHPlayerController* Controller) override;
	virtual EControlMode GetMode() const override;
};


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHPlayerControllerTurningMode : public FUHPlayerControllerMode
{
	GENERATED_BODY()
	
	virtual void Enter(AUHPlayerController* Controller) override;
	virtual void Leave(AUHPlayerController* Controller) override;
	virtual EControlMode GetMode() const override;
};


UCLASS()
class ULTRAHANDMECHANICS_API AUHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UUHPicker* Picker;

	UPROPERTY(EditAnywhere)
	FUHPlayerControllerRegularMode RegularMode;
	
	UPROPERTY(EditAnywhere)
	FUHPlayerControllerPickingMode PickingMode;
	
	UPROPERTY(EditAnywhere)
	FUHPlayerControllerManipulatingMode ManipulatingMode;
	
	UPROPERTY(EditAnywhere)
	FUHPlayerControllerTurningMode TurningMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandStartAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandPickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandStopAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandLookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandMoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnStartAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnStopAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnLeftAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnRightAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnUpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandTurnDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandAttachAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UltraHandDetachAction;

	UPROPERTY(EditAnywhere)
	float MaxRotationSpeed;

	UPROPERTY(EditAnywhere)
	float RotationCorrectionDelay;

	UPROPERTY(EditAnywhere)
	float RotationCorrectionSpeed;

	UPROPERTY(EditAnywhere)
	float MaxYawOffset;

	UPROPERTY(EditAnywhere)
	float MaxDistanceOffset;
	
	AUHPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UFUNCTION(BlueprintCallable)
	EControlMode GetControlMode() const;

	UUHManipulator* GetPawnManipulator() const;
	
protected:
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* Pawn) override;
	virtual void OnUnPossess() override;

private:
	void Jump();
	void StopJumping();
	void UltraHandStart();
	void UltraHandPick();
	void UltraHandStop();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void UltraHandMove(const FInputActionValue& Value);
	void UltraHandLook(const FInputActionValue& Value);
	void UltraHandTurnStart();
	void UltraHandTurnStop();
	void UltraHandTurnLeft();
	void UltraHandTurnRight();
	void UltraHandTurnUp();
	void UltraHandTurnDown();
	void UltraHandAttach();
	void UltraHandDetach();

	void EnterMode(FUHPlayerControllerMode* Mode);
	void LeaveMode(FUHPlayerControllerMode* Mode);
	void LeaveModesUpTo(FUHPlayerControllerMode* Mode);
	void SyncCameraWithCurrentMode();

	void EnforceMaxRotationSpeed(float DeltaTime);
	void EnforceMaxYawOffset();
	
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;
	AUHCharacter* GetUltraHandCharacter() const;

	float DeviationBasedMovementScale(float LocalHeadingAngle) const;

	UPROPERTY(Transient)
	UUHManipulator* PawnManipulator;

	float TimeSinceLastUltraHandInput;

	TArray<FUHPlayerControllerMode*> ModeStack;
};
