#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHBlock.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRAHANDMECHANICS_API UUHBlock : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUHBlock();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
