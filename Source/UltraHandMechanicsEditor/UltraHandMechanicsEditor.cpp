#include "UltraHandMechanicsEditor.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "UHAttachableVisualizer.h"
#include "UltraHandMechanics/UHAttachable.h"

IMPLEMENT_GAME_MODULE(FUltraHandMechanicsEditorModule, UltraHandMechanicsEditor);

void FUltraHandMechanicsEditorModule::StartupModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(
			UUHAttachable::StaticClass()->GetFName(),
			MakeShareable(new FUHAttachableVisualizer));
	}
}

void FUltraHandMechanicsEditorModule::ShutdownModule()
{
	if (GUnrealEd) 
	{
		GUnrealEd->UnregisterComponentVisualizer(UUHAttachable::StaticClass()->GetFName());
	}
}
