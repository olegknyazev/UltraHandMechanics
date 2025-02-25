#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHPicker.generated.h"


class UUHBlock;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHPicker : public UActorComponent
{
	GENERATED_BODY()

public:
	UUHPicker();

	UPROPERTY()
	UUHBlock* SelectedBlock;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};
