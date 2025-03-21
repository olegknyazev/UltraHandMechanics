#pragma once

#include "ComponentVisualizer.h"

class ULTRAHANDMECHANICSEDITOR_API FUHAttachableVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(
		const UActorComponent* Component,
		const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
};
