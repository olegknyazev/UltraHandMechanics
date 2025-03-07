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
	BlockComponent->SetHighlighted(Component != nullptr);
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
	
	USceneComponent* const OldRoot = GetRootComponent();
	if (OldRoot == NewRoot)
	{
		return;
	}
	
	UPrimitiveComponent* const OldRootPrimitive = Cast<UPrimitiveComponent>(OldRoot);
	UPrimitiveComponent* const NewRootPrimitive = Cast<UPrimitiveComponent>(NewRoot);

	OldRootPrimitive->SetSimulatePhysics(false);
	NewRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetRootComponent(NewRoot);
	OldRoot->AttachToComponent(NewRoot, FAttachmentTransformRules::KeepWorldTransform);

	OldRootPrimitive->WeldTo(NewRoot);

	MovementComponent->SetUpdatedComponent(NewRoot);
	
	BlockComponent->SetPrimitiveComponent(NewRootPrimitive);
}

void AUHBaseBlock::BeginPlay()
{
	Super::BeginPlay();
}

void AUHBaseBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
