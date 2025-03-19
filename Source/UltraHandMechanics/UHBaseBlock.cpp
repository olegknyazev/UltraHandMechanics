#include "UHBaseBlock.h"

#include "UHAttachable.h"
#include "UHBlockHighlight.h"
#include "UHBlockMovementComponent.h"


AUHBaseBlock::AUHBaseBlock()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetSimulatePhysics(true);
	RootComponent = MeshComponent;

	MovementComponent = CreateDefaultSubobject<UUHBlockMovementComponent>(TEXT("Movement"));
	MovementComponent->UpdatedComponent = MeshComponent;
	MovementComponent->AddTickPrerequisiteComponent(HighlightComponent);
	
	AttachableComponent = CreateDefaultSubobject<UUHAttachable>(TEXT("Attachable"));
	AttachableComponent->AttachablePrimitive = MeshComponent;
	AttachableComponent->MovementComponent = MovementComponent;

	HighlightComponent = CreateDefaultSubobject<UUHBlockHighlight>(TEXT("Block"));
	HighlightComponent->SetPrimitiveComponent(MeshComponent);
	HighlightComponent->MovementComponent = MovementComponent;
	HighlightComponent->Attachable = AttachableComponent;
}

void AUHBaseBlock::SetHighlightedPart(UStaticMeshComponent* Component)
{
	if (Component)
	{
		if (!ensure(Component->GetOwner() == this))
		{
			return;
		}
		HighlightComponent->SetPrimitiveComponent(Component);
	}
	HighlightComponent->SetUnderAim(Component != nullptr);
}

bool AUHBaseBlock::IsManipulated() const
{
	return bManipulated;
}

void AUHBaseBlock::SetManipulated(bool bInManipulated)
{
	if (bManipulated != bInManipulated)
	{
		bManipulated = bInManipulated;
		
		if (!bManipulated)
		{
			MovementComponent->StopMovementImmediately();
		}

		if (auto* RootPrimitive = Cast<UPrimitiveComponent>(RootComponent))
		{
			RootPrimitive->SetEnableGravity(!bManipulated);
			RootPrimitive->SetPhysMaterialOverride(bManipulated ? ManipulatedPhysicalMaterial : nullptr);
		}

		HighlightComponent->SetManipulated(bManipulated);
	}
}

bool AUHBaseBlock::AnyPartHighlighted() const
{
	return HighlightComponent->IsUnderAim();
}

void AUHBaseBlock::Reroot(USceneComponent* NewRoot)
{
	if (!ensure(NewRoot))
	{
		return;
	}

	if (!ensure(NewRoot->GetOwner() == this))
	{
		return;
	}
	
	auto* const NewRootPrimitive = Cast<UPrimitiveComponent>(NewRoot);
	if (!ensure(NewRootPrimitive))
	{
		return;
	}
	
	if (!ensure(!bManipulated))
	{
		return;
	}
	
	auto* const OldRootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent());
	check(OldRootPrimitive);
	
	if (OldRootPrimitive == NewRootPrimitive)
	{
		return;
	}

	OldRootPrimitive->SetSimulatePhysics(false);
	NewRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetRootComponent(NewRoot);
	OldRootPrimitive->AttachToComponent(NewRoot, FAttachmentTransformRules::KeepWorldTransform);
	OldRootPrimitive->WeldTo(NewRoot);
	NewRootPrimitive->SetSimulatePhysics(true);

	MovementComponent->SetUpdatedComponent(NewRoot);
	
	HighlightComponent->SetPrimitiveComponent(NewRootPrimitive);
}

bool AUHBaseBlock::StartSticking()
{
	if (AttachableComponent->StartSticking())
	{
		if (bManipulated)
		{
			bManipulated = false;
			MovementComponent->StopMovementImmediately();
			HighlightComponent->SetManipulated(false);
		}
		
		return true;
	}

	return false;
}

void AUHBaseBlock::BeginPlay()
{
	Super::BeginPlay();
}

void AUHBaseBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUHBaseBlock::StartAttaching(float InMaxAttachDistance)
{
	AttachableComponent->StartAttaching(InMaxAttachDistance);
}

void AUHBaseBlock::StopAttaching()
{
	AttachableComponent->StopAttaching();
}

void AUHBaseBlock::Detach(USceneComponent* PartToDetach)
{
	AttachableComponent->Detach(PartToDetach);
}
