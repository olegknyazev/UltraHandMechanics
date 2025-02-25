#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "UHManipulator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHManipulator : public USceneComponent
{
	GENERATED_BODY()

public:	
	UUHManipulator();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
