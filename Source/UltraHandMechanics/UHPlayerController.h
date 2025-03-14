#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UHPlayerController.generated.h"

class AUHCharacter;
class UUHPicker;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS()
class ULTRAHANDMECHANICS_API AUHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UUHPicker* Picker;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* UltraHandPickingMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* UltraHandManipulatingMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* UltraHandManipulatingTurningMappingContext;
	
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
	
	AUHPlayerController();
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	virtual void SetupInputComponent() override;

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

	AUHCharacter* GetUltraHandCharacter() const;
};
