// 

#pragma once

#include "CoreMinimal.h"
#include "Containers/Deque.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "StevesPooledActorSystem.generated.h"

/**
 * General actor pooling system
 */
UCLASS()
class STEVESUEHELPERS_API UStevesPooledActorSystem : public UWorldSubsystem
{
	GENERATED_BODY()
protected:
	TMap<UClass*, TDeque<TObjectPtr<AActor>>> Pools;

	/// The location that actors in the pool are stored at
	UPROPERTY(BlueprintReadWrite)
	FVector StorageLocation = FVector(0,0,-100000);

	TDeque<TObjectPtr<AActor>>* GetPool(UClass* Class, bool bCreate);
	void DisableActor(AActor* Actor);
	void ReviveActor(AActor* Actor, const FVector& Location, const FRotator& Rotator);

	AActor* SpawnNewActor(UClass* ActorClass, const FVector& Location, const FRotator& Rotation);
	void DrainPool(TDeque<TObjectPtr<AActor>>* pPool, int NumberToKeep);

public:

	UFUNCTION(BlueprintCallable, Category="Pooling")
	static UStevesPooledActorSystem* Get(const UObject* WorldContext);

	/// Get the location that actors in the pool are stored at
	const FVector& GetStorageLocation() const
	{
		return StorageLocation;
	}

	/// Set the location that actors are stored at when in the pool
	void SetStorageLocation(const FVector& Loc)
	{
		this->StorageLocation = Loc;
	}

	/**
	 * Add an actor to the pool. Will make the actor invisible, teleport it to the storage location, and disable all
	 * physics. You must NOT destroy this actor!
	 * @param Actor 
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	void AddActorToPool(AActor* Actor);
	/**
	 * Release an actor back to the pool, instead of destroying it. Will make the actor invisible, teleport it to a
	 * far away location, and disable physics. You must NOT destroy this actor!
	 * @param Actor 
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	void ReleasePooledActor(AActor* Actor);

	/**
	 * Re-use or spawn an actor of a given class. Note that if the actor is re-used, you will need to re-enable physics
	 * yourself since we do not store that state or assume it will be the same. It will be teleported to the correct location and set visible.
	 * @param ActorClass The class of the actor
	 * @param Location Location in the world
	 * @param Rotation Rotation in the world
	 * @param bWasReUsed This is set to true if the actor was re-used from the pool, and therefore will need to have its
	 *	physics reset by the caller (if not done by an IStevesPooledActor implementation)
	 * @return Spawned or re-used actor. Remember to check physics settings, both will be disabled if re-used.
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	AActor* GetPooledActor(TSubclassOf<AActor> ActorClass,
	                       const FVector& Location,
	                       const FRotator& Rotation,
	                       bool& bWasReUsed);
	
	
	template< class T >
	T* GetPooledActor(UClass* Class, const FVector& Location, const FRotator& Rotation, bool& bOutWasReused)
	{
		return Cast<T>(GetPooledActor(Class, Location, Rotation, bOutWasReused));
	}

	template< class T >
	T* GetPooledActor(UClass* Class, const FVector& Location, const FRotator& Rotation)
	{
		bool Dummy;
		return Cast<T>(GetPooledActor(Class, Location, Rotation, Dummy));
	}

	/**
	 * Pre-warm the actor pool by creating a number of instances and making them invisible
	 * @param ActorClass The class of actors to pre-warm the pool with
	 * @param Count The number of actors to pre-create
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	void PreWarmActorPool(TSubclassOf<AActor> ActorClass, int Count = 20);

	/**
	 * Drain the actor pool for a single class of actor, destroying any surplus actors.
	 * @param ActorClass The actor class to drain the pool for
	 * @param NumberToKeep The number of actors to keep, if any
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	void DrainActorPool(TSubclassOf<AActor> ActorClass, int NumberToKeep = 0);

	/**
	 * Drain all actor pools, destroying all pooled actors.
	 */
	UFUNCTION(BlueprintCallable, Category="Pooling")
	void DrainAllActorPools();
	
	virtual void Deinitialize() override;

};
