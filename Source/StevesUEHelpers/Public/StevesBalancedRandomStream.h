// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#pragma once

#include "CoreMinimal.h"
#include "Math/Halton.h"
#include "StevesBalancedRandomStream.generated.h"

/// "Balanced" random stream, using the Halton Sequence
/// This is deterministic and more uniform in appearance than a general random stream (although not perfectly uniform)
/// NOTE: You cannot generate more than about 500 unique values with this sequence until it begins to repeat
/// This is a limitation of the Halton sequence; rather than lose distribution we loop after this
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesBalancedRandomStream
{
	GENERATED_BODY()

protected:
	int32 InitialSeed = 0;
	mutable uint32 Seed = 0;

	static int32 SafeSeed(int32 InSeed)
	{
		// Halton sequence gets unstable when seed gets too high, especially with higher bases
		constexpr int32 SafeMaxSeed = 500;
		while (InSeed > SafeMaxSeed)
		{
			InSeed -= SafeMaxSeed;
		}
		return InSeed;
	}
	int32 SafeSeedInc() const
	{
		Seed = SafeSeed(Seed + 1);
		return Seed;
	}

public:

	FStevesBalancedRandomStream()
		: InitialSeed(0)
		, Seed(0)
	{ }

	/**
	 * Creates and initializes a new random stream from the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	FStevesBalancedRandomStream( int32 InSeed )
	{ 
		Initialize(InSeed);
	}

	/**
	 * Creates and initializes a new random stream from the specified name.
	 *
	 * @note If NAME_None is provided, the stream will be seeded using the current time.
	 * @param InName The name value from which the stream will be initialized.
	 */
	FStevesBalancedRandomStream( FName InName )
	{
		Initialize(InName);
	}

	/**
	 * Initializes this random stream with the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	void Initialize( int32 InSeed )
	{
		InitialSeed = SafeSeed(InSeed);
		Seed = uint32(InitialSeed);
	}

	/**
	 * Initializes this random stream using the specified name.
	 *
	 * @note If NAME_None is provided, the stream will be seeded using the current time.
	 * @param InName The name value from which the stream will be initialized.
	 */
	void Initialize( FName InName )
	{
		uint32 StartSeed;
		if (InName != NAME_None)
		{
			StartSeed = GetTypeHash(InName.ToString());
		}
		else
		{
			StartSeed = FPlatformTime::Cycles();
		}

		InitialSeed = SafeSeed(StartSeed);
		Seed = InitialSeed;
	}

	/**
	 * Resets this random stream to the initial seed value.
	 */
	void Reset() const
	{
		Seed = uint32(InitialSeed);
	}	

	int32 GetInitialSeed() const
	{
		return InitialSeed;
	}

	/**
	 * Generates a new random seed.
	 */
	void GenerateNewSeed()
	{
		Initialize(SafeSeed(FMath::Rand()));
	}


	/// Return a value between 0..1, inclusive
	float Rand() const
	{
		return Halton(SafeSeedInc(), 2);
	}

	/// Return a 2D value with each element between 0..1, inclusive
	/// Use this rather than calling Rand() twice to ensure balanced distribution
	FVector2D Rand2D() const
	{
		const FVector2D Result(
			Halton(Seed, 2),
			Halton(Seed, 3));
		
		SafeSeedInc();
		
		return Result;
	}

	/// Return a 3D value with each element between 0..1, inclusive
	/// Use this rather than calling Rand() twice to ensure balanced distribution
	FVector Rand3D() const
	{
		const FVector Result(
			Halton(Seed, 2),
			Halton(Seed, 3),
			Halton(Seed, 5));
		SafeSeedInc();
		return Result;
	}

	/**
	 * Returns a random vector of unit size.
	 *
	 * @return Random unit vector.
	 */
	FVector RandUnitVector() const
	{
		const FVector2D PitchYaw = Rand2D();
		return FRotator(PitchYaw.X, PitchYaw.Y, 0).RotateVector(FVector::UpVector);
	}

	/// Random point in a 3D box
	FORCEINLINE FVector RandPointInBox(const FBox& Box) const
	{
		const FVector R3 = Rand3D();
		return FVector(FMath::Lerp(Box.Min.X, Box.Max.X, R3.X),
		               FMath::Lerp(Box.Min.Y, Box.Max.Y, R3.Y),
		               FMath::Lerp(Box.Min.Z, Box.Max.Z, R3.Z));
	}

	/// Random point in a 2D rectangle
	FORCEINLINE FVector2D RandPointInBox2D(const FBox2D& Rect) const
	{
		const FVector2D R2 = Rand2D();
		return FVector2D(FMath::Lerp(Rect.Min.X, Rect.Max.X, R2.X),
					   FMath::Lerp(Rect.Min.Y, Rect.Max.Y, R2.Y));
	}
	
	/// Random point in a circle
	FORCEINLINE FVector2D RandPointInCircle(float Radius = 1.0) const
	{
		// Just use rejection sampling for simplicity / speed
		while (true)
		{
			const FVector2D Candidate = Rand2D();
			if (Candidate.SquaredLength() <= 1.0)
			{
				return Candidate * Radius;
			}
		}
	}

	/// Random point in a sphere
	FORCEINLINE FVector RandPointInSphere(float Radius = 1.0) const
	{
		// Just use rejection sampling for simplicity / speed
		while (true)
		{
			const FVector Candidate = Rand3D();
			if (Candidate.SquaredLength() <= 1.0)
			{
				return Candidate * Radius;
			}
		}
	}

	/// Random value in a range (inclusive)
	float RandRange(float Min, float Max) const
	{
		return FMath::Lerp(Min, Max, Rand());
	}

	/// Random colour value
	FLinearColor RandColour(const FLinearColor& From, const FLinearColor& To) const
	{
		return FLinearColor::LerpUsingHSV(From, To, Rand());
	}
	
	/**
	 * Gets the current seed.
	 *
	 * @return Current seed.
	 */
	int32 GetCurrentSeed() const
	{
		return int32(InitialSeed);
	}
	
	
	FString ToString() const
	{
		return FString::Printf(TEXT("FStevesBalancedRandomStream(InitialSeed=%i, Seed=%u)"), InitialSeed, InitialSeed);
	}	

	
};
