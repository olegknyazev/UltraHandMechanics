#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "UHBlockMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHBlockMovementComponent : public UMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector AngularVelocity;

	UPROPERTY(EditAnywhere)
	float MaxAngularSpeed;
	
	UUHBlockMovementComponent();

	virtual void BeginPlay() override;
	
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	virtual void StopMovementImmediately() override;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	void UpdatedComponentShapeMightChange();
	
private:
	FVector ClampAngularVelocity(const FVector& AngularVelocity) const;

	void UpdateBlockRadius();
	
	float BlockRadius;
};
