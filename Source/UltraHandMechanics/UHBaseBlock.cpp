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

	BlockComponent = CreateDefaultSubobject<UUHBlock>(TEXT("Block"));
	BlockComponent->HighlightablePrimitive = MeshComponent;
	BlockComponent->MovementComponent = MovementComponent;
	
	AttachableComponent = CreateDefaultSubobject<UUHAttachable>(TEXT("Attachable"));
}

void AUHBaseBlock::BeginPlay()
{
	Super::BeginPlay();
}

void AUHBaseBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
