#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UHBaseBlock.generated.h"

class UUHBlock;
class UUHBlockMovementComponent;

UCLASS()
class ULTRAHANDMECHANICS_API AUHBaseBlock : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere)
	UUHBlockMovementComponent* MovementComponent;
	
	UPROPERTY(VisibleAnywhere)
	UUHBlock* BlockComponent;
	
	AUHBaseBlock();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
