// Copyright Steve Streeting
// Licensed under the MIT License (see License.txt)
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "StevesReplicatedPhysicsActor.generated.h"

/// A static mesh actor which is simulated on the server and replicated to clients.
/// The results will be pretty smooth on the clients due to a very specific set of settings.
/// You MUST spawn this actor ONLY on the server!
UCLASS(Blueprintable)
class STEVESUEHELPERS_API AStevesReplicatedPhysicsActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AStevesReplicatedPhysicsActor(const FObjectInitializer& ObjInit);

protected:
	virtual void BeginPlay() override;
	bool IsServer() const;
};
