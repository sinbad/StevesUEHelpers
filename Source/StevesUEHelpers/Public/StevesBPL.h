// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StevesMathHelpers.h"
#include "StevesBPL.generated.h"

/**
 * Blueprint library exposing various things in a Blueprint-friendly way e.g. using by-value FVectors so they can
 * be entered directly if required, rather than const& as in C++. Also use degrees not radians.
 */
UCLASS()
class STEVESUEHELPERS_API UStevesBPL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* Return whether a sphere overlaps a cone
	* @param ConeOrigin Origin of the cone
	* @param ConeDir Direction of the cone, must be normalised
	* @param ConeHalfAngle Half-angle of the cone, in degrees
	* @param Distance Length of the cone
	* @param SphereCentre Centre of the sphere
	* @param SphereRadius Radius of the sphere
	* @return True if the sphere overlaps the cone
	*/
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Math")
	static bool SphereOverlapCone(FVector ConeOrigin, FVector ConeDir, float ConeHalfAngle, float Distance, FVector SphereCentre, float SphereRadius)
	{
		return StevesMathHelpers::SphereOverlapCone(ConeOrigin, ConeDir, FMath::DegreesToRadians(ConeHalfAngle), Distance, SphereCentre, SphereRadius);
	}
	
};
