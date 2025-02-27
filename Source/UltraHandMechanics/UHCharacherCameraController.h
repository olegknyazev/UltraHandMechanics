#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHCharacherCameraController.generated.h"


class UCameraComponent;
class USpringArmComponent;

USTRUCT()
struct ULTRAHANDMECHANICS_API FUH3rdPersonCameraSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector Offset;

	UPROPERTY(EditAnywhere)
	float Distance;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHCharacherCameraController : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USpringArmComponent* SpringArm;

	UPROPERTY()
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	FUH3rdPersonCameraSettings RegularSettings;

	UPROPERTY(EditAnywhere)
	FUH3rdPersonCameraSettings UltraHandSettings;

	UPROPERTY(EditAnywhere)
	float BlendSpeed = 1.f;

	UUHCharacherCameraController();

	void ActivateRegularMode();
	void ActivateUltraHandMode();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class EMode
	{
		Regular,
		UltraHand
	};

	const FUH3rdPersonCameraSettings& GetSettings(EMode Mode) const;

	EMode Mode = EMode::Regular;
};
