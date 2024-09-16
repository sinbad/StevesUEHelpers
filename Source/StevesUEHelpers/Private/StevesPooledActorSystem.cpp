// 


#include "StevesPooledActorSystem.h"

#include "StevesPooledActor.h"

UStevesPooledActorSystem* UStevesPooledActorSystem::Get(const UObject* WorldContext)
{
	if (IsValid(WorldContext))
	{
		if (auto World = WorldContext->GetWorld())
		{
			return World->GetSubsystem<UStevesPooledActorSystem>();
		}
	}
	
	return nullptr;
}

TDeque<TObjectPtr<AActor>>* UStevesPooledActorSystem::GetPool(UClass* Class, bool bCreate)
{
	auto pPool = Pools.Find(Class);
	if (!pPool && bCreate)
	{
		// Create new pool
		pPool = &Pools.Emplace(Class);
	}

	return pPool;
}

void UStevesPooledActorSystem::AddActorToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	UClass* Class = Actor->GetClass()->GetAuthoritativeClass();

	if (auto pPool = GetPool(Class, true))
	{
		pPool->PushLast(Actor);
		DisableActor(Actor);
	}
}

void UStevesPooledActorSystem::ReleasePooledActor(AActor* Actor)
{
	// This is really just a synonym so that Get/Release pattern is more pleasingly symmetrical
	// We have AddActorToPool for externally created actors
	AddActorToPool(Actor);
}

AActor* UStevesPooledActorSystem::GetPooledActor(TSubclassOf<AActor> ActorClass,
                                               FVector const& Location,
                                               FRotator const& Rotation,
                                               bool& bWasReused)
{
	UClass* Class = ActorClass->GetAuthoritativeClass();

	if (auto pPool = GetPool(Class, false))
	{
		// We use the end of the deque meaning we get most recently used actor for better potential cache use
		TObjectPtr<AActor> Ret = nullptr;
		// In case someone has destroyed the actors we've got in the pool, keep trying if not valid until empty
		while (pPool->TryPopLast(Ret))
		{
			if (IsValid(Ret.Get()))
			{
				ReviveActor(Ret.Get(), Location, Rotation);
				bWasReused = true;
				return Ret.Get();
			}
		}
	}

	// We need to spawn a new one
	// We don't add this to the pool of course, because the caller is getting it
	bWasReused = false;
	return SpawnNewActor(Class, Location, Rotation);
}

void UStevesPooledActorSystem::PreWarmActorPool(TSubclassOf<AActor> ActorClass,
                                              int Count)
{
	UClass* Class = ActorClass->GetAuthoritativeClass();
	if (auto pPool = GetPool(Class, true))
	{
		for (int i = pPool->Num(); i < Count; ++i)
		{
			auto Actor = SpawnNewActor(Class, StorageLocation, FRotator::ZeroRotator);
			DisableActor(Actor);
			pPool->PushLast(TObjectPtr<AActor>(Actor));
		}
	}
}

AActor* UStevesPooledActorSystem::SpawnNewActor(UClass* Class, const FVector& Location, const FRotator& Rotation)
{
	FActorSpawnParameters Params;
	Params.Name = FName(FString::Printf(TEXT("Pooled_%s"), *Class->GetName()));
	Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AActor* Ret = GetWorld()->SpawnActor(Class, &Location, &Rotation, Params);
#if WITH_EDITOR
	Ret->SetActorLabel(Ret->GetName());
#endif
	return Ret;
}

void UStevesPooledActorSystem::DisableActor(AActor* Actor)
{
	if (Actor)
	{
		Actor->SetActorHiddenInGame(true);
		if (auto Prim = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
		{
			Prim->SetSimulatePhysics(false);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
			Prim->ResetSceneVelocity();
#endif
		}
		Actor->SetActorLocation(StorageLocation, false, nullptr, ETeleportType::ResetPhysics);

		if (Actor->Implements<UStevesPooledActor>())
		{
			IStevesPooledActor::Execute_DeactivateOnAddedToPool(Actor);
		}
	}
}

void UStevesPooledActorSystem::ReviveActor(AActor* Actor, const FVector& Location, const FRotator& Rotator)
{
	if (Actor)
	{
		Actor->SetActorHiddenInGame(false);
		// We don't enable physics, because caller may not want that. 
		Actor->SetActorLocation(Location, false, nullptr, ETeleportType::ResetPhysics);
		Actor->SetActorRotation(Rotator, ETeleportType::ResetPhysics);
		
		if (Actor->Implements<UStevesPooledActor>())
		{
			IStevesPooledActor::Execute_ReactivateOnRemovedFromPool(Actor);
		}
	}
}

void UStevesPooledActorSystem::DrainActorPool(TSubclassOf<AActor> ActorClass, int NumberToKeep)
{
	UClass* Class = ActorClass->GetAuthoritativeClass();
	if (auto pPool = GetPool(Class, false))
	{
		DrainPool(pPool, NumberToKeep);
	}
}

void UStevesPooledActorSystem::DrainPool(TDeque<TObjectPtr<AActor>>* pPool, int NumberToKeep)
{
	while (pPool->Num() > NumberToKeep)
	{
		TObjectPtr<AActor> Actor = nullptr;
		if(pPool->TryPopLast(Actor))
		{
			Actor->Destroy();
		}
		else
		{
			// Should never happen but just in case, prevent infinite loop
			break;
		}
	}
}

void UStevesPooledActorSystem::DrainAllActorPools()
{
	for (auto& Pair : Pools)
	{
		DrainPool(&Pair.Value, 0);
	}
}

void UStevesPooledActorSystem::Deinitialize()
{
	Super::Deinitialize();

	DrainAllActorPools();
}
