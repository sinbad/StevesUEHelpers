#pragma once
#include "CoreMinimal.h"
#include "StevesPooledActor.generated.h"

UINTERFACE(MinimalAPI)
class UStevesPooledActor : public UInterface
{
	GENERATED_BODY()
};


/**
 * Optional interface for pooled actors to implement. You don't have to do this, but if you implement this interface
 * for actors you add/remove from UStevesPooledActorSystem, you get additional calls to let you know when that actor is
 * being added or removed from the pool.
 */
class STEVESUEHELPERS_API IStevesPooledActor
{
	GENERATED_BODY()
public:
	
	/**
	 * This function is called when this actor is added to the pool. Implement this if you want to perform actions to disable the
	 * actor other than the default ones (hiding the actor, disabling collision / physics on the root component, moving to a storage location).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pooling")
	void DeactivateOnAddedToPool();

	/**
	 * This function is called when the actor is removed from the pool, to be re-used again. Implement this to re-activate
	 * or reset the actor; the default behaviour is just to unhide the actor (collision / physics are not re-enabled automatically).
	 * You could also do this in the caller to UStevesPooledActorSystem::GetPooledActor, but this interface is here if
	 * you want to do it on the object itself.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Pooling")
	void ReactivateOnRemovedFromPool();

};