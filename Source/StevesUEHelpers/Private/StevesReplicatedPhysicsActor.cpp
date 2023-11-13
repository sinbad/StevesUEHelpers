// Copyright Steve Streeting
// Licensed under the MIT License (see License.txt)
#include "StevesReplicatedPhysicsActor.h"


AStevesReplicatedPhysicsActor::AStevesReplicatedPhysicsActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bStaticMeshReplicateMovement = true;

	const auto MeshComp = GetStaticMeshComponent();
	MeshComp->SetMobility(EComponentMobility::Movable);
	MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComp->bReplicatePhysicsToAutonomousProxy = true;

	// We do NOT replicate MeshComp itself! That's expensive and unnecessary

}

void AStevesReplicatedPhysicsActor::BeginPlay()
{
	SetReplicateMovement(true);

	const auto MeshComp = GetStaticMeshComponent();

	// Server test, includes dedicated and listen servers
	if (IsServer())
	{
		// Server collision. Block all but allow pawns through
		// Subclasses can change this if they like
		MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		MeshComp->SetSimulatePhysics(true);
	}
	else
	{
		// Ignore all collisions on client
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	
	Super::BeginPlay();	
}

bool AStevesReplicatedPhysicsActor::IsServer() const
{
	// Server test, includes dedicated and listen servers
	// I prefer this to Authority test because you can't replicate objects client->server or client->client anyway
	return GetWorld()->GetNetMode() < NM_Client;
}
