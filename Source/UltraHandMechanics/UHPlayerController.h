// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UHPlayerController.generated.h"

class UUHPicker;

/**
 * 
 */
UCLASS()
class ULTRAHANDMECHANICS_API AUHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UUHPicker* Picker;
	
	AUHPlayerController();
	
	virtual void Tick(float DeltaSeconds) override;
};
