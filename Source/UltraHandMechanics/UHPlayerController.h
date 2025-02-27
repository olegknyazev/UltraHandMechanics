// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UHPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UUHPicker;

/**
 * 
 */
UCLASS()
class ULTRAHANDMECHANICS_API AUHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UUHPicker* Picker;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* UltraHandPickingMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* UltraHandManipulatingMappingContext;
	
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
	
	AUHPlayerController();
	
	virtual void Tick(float DeltaSeconds) override;

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
};
