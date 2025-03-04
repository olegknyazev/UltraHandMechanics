#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHAttachable.generated.h"


USTRUCT()
struct ULTRAHANDMECHANICS_API FUHAttachmentSocket
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector Location;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULTRAHANDMECHANICS_API UUHAttachable : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FUHAttachmentSocket> Sockets;
	
	UUHAttachable();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};
