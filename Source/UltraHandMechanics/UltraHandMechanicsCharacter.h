// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "UltraHandMechanicsCharacter.generated.h"

class UUHCharacherCameraController;
class UUHManipulator;
class USpringArmComponent;
class UCameraComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AUltraHandMechanicsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	UUHManipulator* Manipulator;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UUHCharacherCameraController* CameraController;
	
	AUltraHandMechanicsCharacter();

	void ActivateRegularCamera();
	void ActivateUltraHandCamera();
};

