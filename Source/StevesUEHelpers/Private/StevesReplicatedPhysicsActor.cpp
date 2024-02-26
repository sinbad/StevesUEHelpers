// Copyright Steve Streeting
// Licensed under the MIT License (see License.txt)
#include "StevesReplicatedPhysicsActor.h"


AStevesReplicatedPhysicsActor::AStevesReplicatedPhysicsActor(const FObjectInitializer& ObjInit)
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bStaticMeshReplicateMovement = true;

	const auto MeshComp = GetStaticMeshComponent();
	MeshComp->SetMobility(EComponentMobility::Movable);
	MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	// I think this is actually not needed, since on clients the role is ROLE_SimulatedProxy not ROLE_AutonomousProxy
	// When using ROLE_SimulatedProxy physics is replicated all the time
	MeshComp->bReplicatePhysicsToAutonomousProxy = true;

	// The default MinNetUpdateFrequency of 2 is too slow at responding to woken physics objects
	MinNetUpdateFrequency = 10;
	
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
		// We're on a client so will be receiving replicated physics
		// Ignore all collisions; if we don't do this, things jank because of conflicting collisions
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);

		// We DON'T disable physics. Physics still needs to be enabled for replicated physics to be respected.
		// Otherwise objects will not move on the client.

		// There's one case where this is problematic: replicated physics actors that are already asleep
		// once a new client joins a game in-progress will locally simulate themselves through the floor, because no further
		// updates will be received about their physics from the server. 
		// My solution is to start all objects asleep on the client; if they're not, the server will send updates and
		// the objects will wake up. If they're asleep on the server, we're good.
		MeshComp->PutAllRigidBodiesToSleep();
	}
	
	Super::BeginPlay();	
}

bool AStevesReplicatedPhysicsActor::IsServer() const
{
	// Server test, includes dedicated and listen servers
	// I prefer this to Authority test because you can't replicate objects client->server or client->client anyway
	return GetWorld()->GetNetMode() < NM_Client;
}
