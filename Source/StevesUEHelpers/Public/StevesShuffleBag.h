
#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "StevesShuffleBag.generated.h"

/// A shufflebag is an array that contains a certain number of items, and can randomly withdraw one
/// at a time until there are none left. No repeat items will be retrieved until the bag is empty.
/// Requires a type T which is the contained item (keep it small, will be copied around), and optionally
/// TRandStream which must be a type which implements the same interface as FRandomStream but may generate
/// numbers differently.
template<typename T, typename TRandStream = FRandomStream>
struct FStevesShuffleBag
{
protected:
	TArray<T> Bag;
	TRandStream RandomStream;
	// This is the dividing line between items still in the bag and those which have already been pulled.
	// Any index < SentinelIndex is an item in the bag, any >= SentinelIndex have already been pulled.
	int SentinelIndex;
	

public:
	/// Construct using a zero seed & standard capacity
	FStevesShuffleBag()
	{
		Init(0);
	}
	
	/// Construct using a known seed & capacity
	FStevesShuffleBag(int32 Seed, int Capacity = 32)
	{
		Init(Seed);
	}


	/// (Re-)initialise the shuffle bag, emptying its contents and resetting the seed.
	void Init(int32 Seed, int Capacity = 32)
	{
		Bag.Reset(Capacity);
		RandomStream.Initialize(Seed);
		SentinelIndex = -1;
	}
	
	/// Reserve space for a given number of entries
	void Reserve(int Capacity)
	{
		Bag.Reserve(Capacity);
	}
	
	/**
	 * Add one or more copies of an item to the bag.
	 * @param NewItem The new item to add
	 * @param Count The number of copies of this item to add. You can control the probability of items
	 * by adding multiple of the same thing.
	 * @note If you have already started pulling items from the bag, these items will NOT be available
	 * to be pulled until the bag is next re-filled. They will count as having been pulled already
	 * (so preferably add all your items before you start pulling items from the bag)
	 */
	void Add(const T& NewItem, int Count = 1)
	{
		for (int i = 0; i < Count; ++i)
		{
			Bag.Add(NewItem);
		}
	}

	/// Remove one instance of an item from the bag and discard it. This is different to pulling the
	/// item from the bag since it will not re-enter the bag on Reset. The item may have been pulled
	/// already.
	/// Returns whether any item was removed
	bool Remove(const T& Item)
	{
		int Index = Bag.IndexOfByKey(Item);
		if (Index != INDEX_NONE)
		{
			Bag.RemoveAt(Index);
			if (Index < SentinelIndex)
			{
				// This item hadn't been pulled yet
				--SentinelIndex;
			}
			return true;
		}

		return false;
	}

	/// Remove all instance of an item from the bag and discard them. This is different to pulling the
	/// items from the bag since they will not re-enter the bag on Reset.
	/// Returns the number of items removed.
	int RemoveAll(const T& Item)
	{
		int RemovedCount = 0;
		while (Remove(Item))
		{
			++RemovedCount;
		}
		return RemovedCount;
	}
	
	/// Empty the bag, discarding the contents
	void Empty()
	{
		// Keep allocations
		Bag.Reset();
		SentinelIndex = -1;
	}

	/// Reset the bag, returning all previously retrieved contents to the bag
	void Reset()
	{
		// Any items < SentinelIndex are in the bag
		SentinelIndex = Bag.Num();
	}

	/// Pull a random item from those remaining in the bag. If the bag is empty, it will be
	/// refilled using the tokens that were previously pulled (via Reset).
	T Next()
	{
		if (SentinelIndex <= 0)
		{
			// Empty / new bag
			Reset();
			
		}

		// Empty bag
		if (SentinelIndex == 0)
		{
			return T();
		}
		
		// rand max is inclusive, and we need to exclude >= SentinelIndex
		int NextIdx = RandomStream.RandRange(0, SentinelIndex - 1);
		// Copy out
		T Ret = Bag[NextIdx];
		
		--SentinelIndex;
		if (NextIdx != SentinelIndex) {
			// swap the item that's now beyond the sentinel into our slot
			Bag.Swap(NextIdx, SentinelIndex);
		}
		return Ret;
	}

	/// Get the number of items left in the bag
	int GetNumRemaining() const
	{
		return SentinelIndex > 0 ? SentinelIndex : 0;
	}

	/// Get the number of items which have already been pulled from the bag
	int GetNumPulled() const
	{
		return GetNumTotal() - GetNumRemaining();
	}

	/// Get the number of items both remaining in the bag and which have already been pulled from it
	int GetNumTotal() const
	{
		return Bag.Num();
	}

};

/// A shufflebag is an array that contains a certain number of items, and can randomly withdraw one
/// at a time until there are none left. No repeat items will be retrieved until the bag is empty.
/// Because Blueprints can't support templated types, FStevesIndexShuffleBag provides a concretely
/// typed shuffle bag which simply provides array indices rather than values. When you pull out an
/// item, instead of pulling out the *actual* item, you pull out an index into another array.
/// You can therefore use this with any other array so long as you keep them synchronised.
/// If you want to customise the probabilities, add multiple entries of the same item to *your* array
/// before constructing the index shuffle bag.
/// Use MakeIndexShuffleBag to create.
UCLASS(BlueprintType)
class UStevesIndexShuffleBag : public UObject
{
	GENERATED_BODY()

protected:
	FStevesShuffleBag<int> IndexBag;
public:

	UStevesIndexShuffleBag()
	{
	}
	
	/// Reinitialise the shuffle bag with a new size / seed.
	UFUNCTION(BlueprintCallable, Category="Shuffle Bag")
	void Reinitialise(int Count, int32 Seed)
	{
		IndexBag.Init(Seed, Count);

		for (int i = 0; i < Count; ++i)
		{
			IndexBag.Add(i);
		}
	}

	
	/**
	 * Create a new shuffle bag of indexes for accessing items in an array
	 * @param Outer The 
	 * @param Count The number of items in the array you'll be accessing with these indexes
	 * @param Seed The seed for the random number generator
	 * @return The new index shuffle bag
	 */
	UFUNCTION(BlueprintCallable, Category="Shuffle Bag")
	static UStevesIndexShuffleBag* MakeIndexShuffleBag(UObject* Outer, int Count, int32 Seed = 0)
	{
		auto Ret = NewObject<UStevesIndexShuffleBag>(Outer);
		Ret->Reinitialise(Count, Seed);
		return Ret;
	}

	

	/// Get the number of items left in the bag
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shuffle Bag")
	int GetNumRemaining() const
	{
		return IndexBag.GetNumRemaining();
	}

	/// Get the number of items which have already been pulled from the bag
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shuffle Bag")
	int GetNumPulled() const
	{
		return IndexBag.GetNumPulled();
	}

	/// Get the number of items both remaining in the bag and which have already been pulled from it
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Shuffle Bag")
	int GetNumTotal() const
	{
		return IndexBag.GetNumTotal();
	}

	/// Reset the bag, returning all previously retrieved contents to the bag
	UFUNCTION(BlueprintCallable, Category="Shuffle Bag")
	void Reset()
	{
		IndexBag.Reset();
	}

	/// Pull a random index from those remaining in the bag. If the bag is empty, it will be
	/// refilled using the indexes that were previously pulled (via Reset).
	UFUNCTION(BlueprintCallable, Category="Shuffle Bag")
	int Next()
	{
		return IndexBag.Next();
	}
	
	
};

