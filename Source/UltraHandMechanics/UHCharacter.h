#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UHCharacter.generated.h"

class UUHCharacherCameraController;
class UUHManipulator;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(config=Game)
class AUHCharacter : public ACharacter
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

	UPROPERTY(EditAnywhere)
	float WalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float WalkSpeedManipulating;
	
	AUHCharacter();

	void ActivateRegularCamera();
	void ActivateUltraHandPickingCamera();
	void ActivateUltraHandManipulatingCamera(USceneComponent* ManipulatedComponent);

	virtual void Tick(float DeltaSeconds) override;
};

