// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#pragma once

#include "CoreMinimal.h"
#include "Math/Halton.h"
#include "StevesBalancedRandomStream.generated.h"

// Credit to Andrew Wilmott for lots of the algorithms here
// See https://github.com/andrewwillmott/distribute-lib
// Used under Unlicense 

namespace StevesRandConstants
{
	constexpr uint32 kSafeMaxSeed2D = 43046721 - 1;
	constexpr uint32 kSafeMaxSeed3D = 9765625 - 1;
	constexpr float kOneOverThree = 1.0f / 3.0f;
	constexpr float kOneOverFive = 1.0f / 5.0f;

}

/// "Balanced" random stream, using the Halton Sequence
/// This is deterministic and more uniform in appearance than a general random stream (although not perfectly uniform)
/// This is a generic stream which can do 1D, 2D and 3D sequences. If you only need a single type, it's
/// more efficient to use FStevesBalancedRandomStream1D/2D/3D
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesBalancedRandomStream
{
	GENERATED_BODY()

protected:
	uint32 InitialSeed = 0;
	uint32 Seed = 0;
	uint32 Base3Seed = 0;
	uint32 Base5Seed = 0;
	
	static int32 SafeSeed(uint32 InSeed)
	{
		// Halton sequence gets unstable when seed gets too high, especially with higher bases
		while (InSeed > StevesRandConstants::kSafeMaxSeed3D)
		{
			InSeed -= StevesRandConstants::kSafeMaxSeed3D;
		}
		return InSeed;
	}
	
	void UpdateSeeds()
	{
		// For simplicity this version just derives the other seeds from the main one rather than
		// calculating the sequence value at the same time like Andrew's does
		Base3Seed = 0;

		// Average iterations: 1.5
		for (int i = 0, k = Seed; k; i += 2, k /= 3)
		{ 
			const int d = (k % 3);
			Base3Seed |= d << i;
		}

		Base5Seed = 0;

		// Average iterations: 2.5
		for (int i = 0, k = Seed; k; i += 3, k /= 5)
		{ 
			const int d = (k % 5);
			Base5Seed |= d << i;
		}
	}
		
	uint32 SafeSeedInc()
	{
		Seed = SafeSeed(Seed + 1);
		
		UpdateSeeds();
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
	FStevesBalancedRandomStream( uint32 InSeed )
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
	void Initialize( uint32 InSeed )
	{
		InitialSeed = SafeSeed(InSeed);
		Seed = InitialSeed;
		
		UpdateSeeds();
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
		Initialize(StartSeed);
	}

	/**
	 * Resets this random stream to the initial seed value.
	 */
	void Reset()
	{
		Initialize(InitialSeed);
	}	

	uint32 GetInitialSeed() const
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
	float Rand()
	{
		return Halton(SafeSeedInc(), 2);
	}

	/// Return a 2D value with each element between 0..1, inclusive
	/// Use this rather than calling Rand() twice to ensure balanced distribution
	FVector2D Rand2D()
	{
		const float X = Halton(Seed, 2);
		const float Y = Halton(Base3Seed, 3);
		
		SafeSeedInc();
		
		return FVector2D(X, Y);
	}

	/// Return a 3D value with each element between 0..1, inclusive
	/// Use this rather than calling Rand() twice to ensure balanced distribution
	FVector Rand3D()
	{
		const float X = Halton(Seed, 2);
		const float Y = Halton(Base3Seed, 3);
		const float Z = Halton(Base5Seed, 5);
		SafeSeedInc();
		return FVector(X, Y, Z);
	}

	/**
	 * Returns a random vector of unit size.
	 *
	 * @return Random unit vector.
	 */
	FVector RandUnitVector()
	{
		const FVector2D PitchYaw = Rand2D();
		return FRotator(PitchYaw.X, PitchYaw.Y, 0).RotateVector(FVector::UpVector);
	}

	/// Random point in a 3D box
	FORCEINLINE FVector RandPointInBox(const FBox& Box)
	{
		const FVector R3 = Rand3D();
		return FVector(FMath::Lerp(Box.Min.X, Box.Max.X, R3.X),
		               FMath::Lerp(Box.Min.Y, Box.Max.Y, R3.Y),
		               FMath::Lerp(Box.Min.Z, Box.Max.Z, R3.Z));
	}

	/// Random point in a 2D rectangle
	FORCEINLINE FVector2D RandPointInBox2D(const FBox2D& Rect)
	{
		const FVector2D R2 = Rand2D();
		return FVector2D(FMath::Lerp(Rect.Min.X, Rect.Max.X, R2.X),
					   FMath::Lerp(Rect.Min.Y, Rect.Max.Y, R2.Y));
	}
	
	/// Random point in a circle
	FORCEINLINE FVector2D RandPointInCircle(float Radius = 1.0)
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
	FORCEINLINE FVector RandPointInSphere(float Radius = 1.0)
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
	float RandRange(float Min, float Max)
	{
		return FMath::Lerp(Min, Max, Rand());
	}

	/// Random colour value
	FLinearColor RandColour(const FLinearColor& From, const FLinearColor& To)
	{
		return FLinearColor::LerpUsingHSV(From, To, Rand());
	}
	
	/**
	 * Gets the current seed.
	 *
	 * @return Current seed.
	 */
	uint32 GetCurrentSeed() const
	{
		return InitialSeed;
	}
	
	
	FString ToString() const
	{
		return FString::Printf(TEXT("FStevesBalancedRandomStream(InitialSeed=%i, Seed=%u)"), InitialSeed, InitialSeed);
	}	

	
};

/// "Balanced" random stream, using the Halton Sequence, one dimension only (more efficient for this than FStevesBalancedRandomStream)
/// This is deterministic and more uniform in appearance than a general random stream (although not perfectly uniform)
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesBalancedRandomStream1D
{
	GENERATED_BODY()

protected:
	uint32 InitialSeed = 0;
	uint32 Seed = 0;

public:

	FStevesBalancedRandomStream1D()
	{ }

	/**
	 * Creates and initializes a new random stream from the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	FStevesBalancedRandomStream1D( uint32 InSeed )
	{ 
		Initialize(InSeed);
	}

	/**
	 * Creates and initializes a new random stream from the specified name.
	 *
	 * @note If NAME_None is provided, the stream will be seeded using the current time.
	 * @param InName The name value from which the stream will be initialized.
	 */
	FStevesBalancedRandomStream1D( FName InName )
	{
		Initialize(InName);
	}

	/**
	 * Initializes this random stream with the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	void Initialize( uint32 InSeed )
	{
		InitialSeed = Seed = InSeed;
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
		Initialize(StartSeed);
	}

	/**
	 * Resets this random stream to the initial seed value.
	 */
	void Reset()
	{
		Initialize(InitialSeed);
	}	

	uint32 GetInitialSeed() const
	{
		return InitialSeed;
	}

	/**
	 * Generates a new random seed.
	 */
	void GenerateNewSeed()
	{
		Initialize(FMath::Rand());
	}


	/// Return a value between 0..1, inclusive
	float Rand()
	{
		return Halton(Seed++, 2);
	}
	
	/// Random value in a range (inclusive)
	float RandRange(float Min, float Max)
	{
		return FMath::Lerp(Min, Max, Rand());
	}

	/// Random colour value
	FLinearColor RandColour(const FLinearColor& From, const FLinearColor& To)
	{
		return FLinearColor::LerpUsingHSV(From, To, Rand());
	}
	
	/**
	 * Gets the current seed.
	 *
	 * @return Current seed.
	 */
	uint32 GetCurrentSeed() const
	{
		return InitialSeed;
	}
	
	
	FString ToString() const
	{
		return FString::Printf(TEXT("FStevesBalancedRandomStream1D(InitialSeed=%i, Seed=%u)"), InitialSeed, InitialSeed);
	}	
	
};


/// "Balanced" 2D random stream, using the Halton Sequence. More efficient than the general FStevesBalancedRandomStream for 2D work
/// This is deterministic and more uniform in appearance than a general random stream (although not perfectly uniform)
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesBalancedRandomStream2D
{
	GENERATED_BODY()

protected:
	uint32 InitialSeed = 0;
	uint32 Base2Seed = 0;
	uint32 Base3Seed = 0;

	
	FVector2f CurrentValue = FVector2f::ZeroVector;


public:

	FStevesBalancedRandomStream2D()
	{
		Initialize(0);
	}

	/**
	 * Creates and initializes a new random stream from the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	FStevesBalancedRandomStream2D( uint32 InSeed )
	{ 
		Initialize(InSeed);
	}

	/**
	 * Creates and initializes a new random stream from the specified name.
	 *
	 * @note If NAME_None is provided, the stream will be seeded using the current time.
	 * @param InName The name value from which the stream will be initialized.
	 */
	FStevesBalancedRandomStream2D( FName InName )
	{
		Initialize(InName);
	}

	/**
	 * Initializes this random stream with the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	void Initialize( uint32 InSeed )
	{
		// Halton sequence gets unstable when seed gets too high, especially with higher bases
		while (InSeed >= StevesRandConstants::kSafeMaxSeed2D)
		{
			InSeed -= StevesRandConstants::kSafeMaxSeed2D;
		}
		
		InitialSeed = Base2Seed = InSeed;
		
		CurrentValue.X = Halton(Base2Seed, 2);

		CurrentValue.Y = 0;
		Base3Seed = 0;

		float ip = StevesRandConstants::kOneOverThree;
		float p = ip;

		for (int i = 0, k = Base2Seed; k; i += 2, k /= 3)
		{
			int d = (k % 3);
			Base3Seed |= d << i;
			CurrentValue.Y += d * p;
			p *= ip;
		}
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
		Initialize(StartSeed);
	}

	/**
	 * Resets this random stream to the initial seed value.
	 */
	void Reset()
	{
		Initialize(InitialSeed);
	}	

	uint32 GetInitialSeed() const
	{
		return InitialSeed;
	}

	/**
	 * Generates a new random seed.
	 */
	void GenerateNewSeed()
	{
		Initialize(FMath::Rand());
	}

	/// Return a 2D value with each element between 0..1, inclusive
	FVector2f Rand2D()
	{
		// Wrap back to 0 at end of safe range
		if (Base2Seed >= StevesRandConstants::kSafeMaxSeed2D)
		{
			Initialize(0);
		}
		
		// This uses Andrew Wilmott's approach of calculating the next value at the same time as incrementing
		// We calculate the new value while initialising / incrementing, so it's currently correct
		const FVector2f Ret = CurrentValue;
		
		/////////////////////////////////////
		// base 2

		uint32_t OldBase2Seed = Base2Seed;
		Base2Seed++;
		uint32_t Diff = Base2Seed ^ OldBase2Seed;

		// bottom bit always changes, higher bits
		// change less frequently.
		float s = 0.5f;

		// Diff will be of the form 0 * 1 +, i.e. one bits up until the last carry.
		// expected iterations = 1 + 0.5 + 0.25 + ... = 2
		do
		{
			if (OldBase2Seed & 1)
				CurrentValue.X -= s;
			else
				CurrentValue.X += s;

			s *= 0.5f;

			Diff = Diff >> 1;
			OldBase2Seed = OldBase2Seed >> 1;
		}
		while (Diff);


		/////////////////////////////////////
		// base 3: use 2 bits for each base 3 digit.

		uint32_t Mask = 0x3;  // also the max base 3 digit
		uint32_t Add  = 0x1;  // amount to Add to force carry once digit==3
		s = StevesRandConstants::kOneOverThree;

		Base3Seed++;

		// expected iterations: 1.5
		while (true)
		{
			if ((Base3Seed & Mask) == Mask)
			{
				Base3Seed += Add;          // force carry into next 2-bit digit
				CurrentValue.Y -= 2 * s;

				Mask = Mask << 2;
				Add  = Add  << 2;

				s *= StevesRandConstants::kOneOverThree;
			}
			else
			{
				CurrentValue.Y += s;     // we know digit n has gone from a to a + 1
				break;
			}
		}
		
		return Ret;
	}
	
	/**
	 * Returns a random vector of unit size.
	 *
	 * @return Random unit vector.
	 */
	FVector RandUnitVector()
	{
		const FVector2f PitchYaw = Rand2D();
		return FRotator(PitchYaw.X, PitchYaw.Y, 0).RotateVector(FVector::UpVector);
	}
	

	/// Random point in a 2D rectangle
	FORCEINLINE FVector2f RandPointInBox2D(const FBox2D& Rect)
	{
		const FVector2f R2 = Rand2D();
		return FVector2f(FMath::Lerp(Rect.Min.X, Rect.Max.X, R2.X),
					   FMath::Lerp(Rect.Min.Y, Rect.Max.Y, R2.Y));
	}
	
	/// Random point in a circle
	FORCEINLINE FVector2f RandPointInCircle(float Radius = 1.0)
	{
		// Just use rejection sampling for simplicity / speed
		while (true)
		{
			const FVector2f Candidate = Rand2D();
			if (Candidate.SquaredLength() <= 1.0)
			{
				return Candidate * Radius;
			}
		}
	}
	
	/**
	 * Gets the current seed.
	 *
	 * @return Current seed.
	 */
	uint32 GetCurrentSeed() const
	{
		return InitialSeed;
	}
	
	
	FString ToString() const
	{
		return FString::Printf(TEXT("FStevesBalancedRandomStream2D(InitialSeed=%i, Seed=%u)"), InitialSeed, InitialSeed);
	}	
};

/// "Balanced" random 3D stream, using the Halton Sequence. Optimised for 3D only, more efficient than FStevesBalancedRandomStream
/// This is deterministic and more uniform in appearance than a general random stream (although not perfectly uniform)
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesBalancedRandomStream3D
{
	GENERATED_BODY()

protected:
	uint32 InitialSeed = 0;
	uint32 Base2Seed = 0;
	uint32 Base3Seed = 0;
	uint32 Base5Seed = 0;
	
	FVector3f CurrentValue = FVector3f::ZeroVector;
public:

	FStevesBalancedRandomStream3D()
	{
		Initialize(0);
	}

	/**
	 * Creates and initializes a new random stream from the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	FStevesBalancedRandomStream3D( uint32 InSeed )
	{ 
		Initialize(InSeed);
	}

	/**
	 * Creates and initializes a new random stream from the specified name.
	 *
	 * @note If NAME_None is provided, the stream will be seeded using the current time.
	 * @param InName The name value from which the stream will be initialized.
	 */
	FStevesBalancedRandomStream3D( FName InName )
	{
		Initialize(InName);
	}

	/**
	 * Initializes this random stream with the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	void Initialize( uint32 InSeed )
	{
		while (InSeed > StevesRandConstants::kSafeMaxSeed3D)
		{
			InSeed -= StevesRandConstants::kSafeMaxSeed3D;
		}
		
		InitialSeed = Base2Seed = InSeed;
		
		CurrentValue.X = Halton(Base2Seed, 2);

		CurrentValue.Y = 0.0f;
		Base3Seed = 0;

		float p = StevesRandConstants::kOneOverThree;

		for (int i = 0, k = Base2Seed; k; i += 2, k /= 3)
		{
			int d = (k % 3);
			Base3Seed |= d << i;
			CurrentValue.Y += d * p;
			p *= StevesRandConstants::kOneOverThree;
		}

		CurrentValue.Z = 0.0f;
		Base5Seed = 0;

		p = StevesRandConstants::kOneOverFive;

		for (int i = 0, k = Base2Seed; k; i += 3, k /= 5)
		{
			int d = (k % 5);
			Base5Seed |= d << i;
			CurrentValue.Z += d * p;
			p *= StevesRandConstants::kOneOverFive;
		}
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
		Initialize(StartSeed);
	}

	/**
	 * Resets this random stream to the initial seed value.
	 */
	void Reset()
	{
		Initialize(InitialSeed);
	}	

	uint32 GetInitialSeed() const
	{
		return InitialSeed;
	}

	/**
	 * Generates a new random seed.
	 */
	void GenerateNewSeed()
	{
		Initialize(FMath::Rand());
	}
	
	/// Return a 3D value with each element between 0..1, inclusive
	FVector Rand3D()
	{
		if (Base2Seed >= StevesRandConstants::kSafeMaxSeed3D)
		{
			Initialize(0);
		}

		// This uses Andrew Wilmott's approach of calculating the next value at the same time as incrementing
		// We calculate the new value while initialising / incrementing, so it's currently correct
		const FVector3f Ret = CurrentValue;
		
		// base 2: 1 bit per digit
		uint32_t OldBase2 = Base2Seed;
		Base2Seed++;
		uint32_t Diff = Base2Seed ^ OldBase2;

		// bottom bit always changes, higher bits
		// change less frequently.
		float s = 0.5f;

		// diff will be of the form 0 * 1 + , i.e. one bits up until the last carry.
		// expected iterations = 1 + 0.5 + 0.25 + ... = 2
		do
		{
			if (OldBase2 & 1)
				CurrentValue.X -= s;
			else
				CurrentValue.X += s;

			s *= 0.5f;

			Diff = Diff >> 1;
			OldBase2 = OldBase2 >> 1;
		}
		while (Diff);


		// base 3: use 2 bits for each base 3 digit.
		uint32_t Mask = 0x3;  // also the max base 3 digit
		uint32_t Add  = 0x1;  // amount to add to force carry once digit==3
		s = StevesRandConstants::kOneOverThree;

		Base3Seed++;

		// expected iterations: 1.5
		while (true)
		{
			if ((Base3Seed & Mask) == Mask)
			{
				Base3Seed += Add;          // force carry into next 2-bit digit
				CurrentValue.Y -= 2 * s;

				Mask = Mask << 2;
				Add  = Add  << 2;

				s *= StevesRandConstants::kOneOverThree;
			}
			else
			{
				CurrentValue.Y += s;     // we know digit n has gone from a to a + 1
				break;
			}
		};

		// base 5: use 3 bits for each base 5 digit.
		Mask = 0x7;
		Add  = 0x3;  // amount to add to force carry once digit==dmax
		uint32_t Dmax = 0x5;  // max digit

		s = StevesRandConstants::kOneOverFive;

		Base5Seed++;

		// expected iterations: 1.25
		while (true)
		{
			if ((Base5Seed & Mask) == Dmax)
			{
				Base5Seed += Add;          // force carry into next 3-bit digit
				CurrentValue.Z -= 4 * s;

				Mask = Mask << 3;
				Dmax = Dmax << 3;
				Add  = Add  << 3;

				s *= StevesRandConstants::kOneOverFive;
			}
			else
			{
				CurrentValue.Z += s;     // we know digit n has gone from a to a + 1
				break;
			}
		};

		return FVector(Ret.X, Ret.Y, Ret.Z);
	}
	

	/// Random point in a 3D box
	FORCEINLINE FVector RandPointInBox(const FBox& Box)
	{
		const FVector R3 = Rand3D();
		return FVector(FMath::Lerp(Box.Min.X, Box.Max.X, R3.X),
		               FMath::Lerp(Box.Min.Y, Box.Max.Y, R3.Y),
		               FMath::Lerp(Box.Min.Z, Box.Max.Z, R3.Z));
	}

	
	/// Random point in a sphere
	FORCEINLINE FVector RandPointInSphere(float Radius = 1.0)
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
	
	/**
	 * Gets the current seed.
	 *
	 * @return Current seed.
	 */
	uint32 GetCurrentSeed() const
	{
		return InitialSeed;
	}
	
	
	FString ToString() const
	{
		return FString::Printf(TEXT("FStevesBalancedRandomStream(InitialSeed=%i, Seed=%u)"), InitialSeed, InitialSeed);
	}	

	
};
