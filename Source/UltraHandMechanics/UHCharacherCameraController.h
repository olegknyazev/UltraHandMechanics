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

	FUH3rdPersonCameraSettings BlendEaseOut(const FUH3rdPersonCameraSettings& Other, float Alpha, float Exp) const;
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
	float BlendTime = 1.f;

	UPROPERTY(EditAnywhere)
	float BlendExponent = 2.f;

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
	void ApplySettings(const FUH3rdPersonCameraSettings& Settings);

	EMode Mode = EMode::Regular;
	EMode NewMode = EMode::Regular;
	float CurrentBlendTime;
};
