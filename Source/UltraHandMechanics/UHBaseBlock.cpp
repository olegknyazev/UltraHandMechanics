#include "UHBaseBlock.h"

#include "UHAttachable.h"
#include "UHBlock.h"
#include "UHBlockMovementComponent.h"


AUHBaseBlock::AUHBaseBlock()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetSimulatePhysics(true);
	RootComponent = MeshComponent;

	MovementComponent = CreateDefaultSubobject<UUHBlockMovementComponent>(TEXT("Movement"));
	MovementComponent->UpdatedComponent = MeshComponent;
	MovementComponent->AddTickPrerequisiteComponent(BlockComponent);
	
	AttachableComponent = CreateDefaultSubobject<UUHAttachable>(TEXT("Attachable"));
	AttachableComponent->AttachablePrimitive = MeshComponent;
	AttachableComponent->MovementComponent = MovementComponent;

	BlockComponent = CreateDefaultSubobject<UUHBlock>(TEXT("Block"));
	BlockComponent->SetPrimitiveComponent(MeshComponent);
	BlockComponent->MovementComponent = MovementComponent;
	BlockComponent->Attachable = AttachableComponent;
}

void AUHBaseBlock::SetHighlightedPart(UStaticMeshComponent* Component)
{
	if (Component)
	{
		if (!ensure(Component->GetOwner() == this))
		{
			return;
		}
		BlockComponent->SetPrimitiveComponent(Component);
	}
	BlockComponent->SetHighlighted(Component != nullptr);
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

		BlockComponent->SetManipulated(bManipulated);
	}
}

bool AUHBaseBlock::AnyPartHighlighted() const
{
	return BlockComponent->IsHighlighted();
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
	
	BlockComponent->SetPrimitiveComponent(NewRootPrimitive);
}

bool AUHBaseBlock::StartSticking()
{
	if (AttachableComponent->StartSticking())
	{
		if (bManipulated)
		{
			bManipulated = false;
			MovementComponent->StopMovementImmediately();
			BlockComponent->SetManipulated(false);
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
