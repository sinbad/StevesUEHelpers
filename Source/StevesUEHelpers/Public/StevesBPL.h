// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StevesBalancedRandomStream.h"

#include "StevesMathHelpers.h"
#include "Components/PanelSlot.h"
#include "StevesBPL.generated.h"

class UPanelWidget;
class UWidget;
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
	* @param ConeAngle Angle of the cone, in degrees
	* @param Distance Length of the cone
	* @param SphereCentre Centre of the sphere
	* @param SphereRadius Radius of the sphere
	* @return True if the sphere overlaps the cone
	*/
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Math")
	static bool SphereOverlapCone(FVector ConeOrigin, FVector ConeDir, float ConeAngle, float Distance, FVector SphereCentre, float SphereRadius)
	{
		return StevesMathHelpers::SphereOverlapCone(ConeOrigin, ConeDir, FMath::DegreesToRadians(ConeAngle*0.5f), Distance, SphereCentre, SphereRadius);
	}


	
	/**
	* Set the focus to a given widget "properly", which means that if this is a widget derived
	* from UFocusableWidget, it calls SetFocusProperly on it which allows a customised implementation.
    * @param Widget The widget to give focus to
	*/
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|UI")
	static void SetWidgetFocus(UWidget* Widget);

	/**
	 * Insert a child widget at a specific index
	 * @param Parent The container widget
	 * @param Child The child widget to add
	 * @param AtIndex The index at which the new child should exist
	 * @returns The slot the child was inserted at
	 */
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|UI")
	static UPanelSlot* InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex = 0);

	UFUNCTION(BlueprintPure, Category="StevesUEHelpers|Random", meta=(NativeMakeFunc))
	static FStevesBalancedRandomStream MakeBalancedRandomStream(int64 Seed);

	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Random")
	static float BalancedRandom(const FStevesBalancedRandomStream& Stream) { return Stream.Rand(); }

	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Random")
	static FVector2D BalancedRandom2D(const FStevesBalancedRandomStream& Stream) { return Stream.Rand2D(); }

	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Random")
	static FVector BalancedRandom3D(const FStevesBalancedRandomStream& Stream) { return Stream.Rand3D(); }

	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Random")
	static FVector BalancedRandomVector(const FStevesBalancedRandomStream& Stream) { return Stream.RandUnitVector(); }

	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Random")
	static FVector BalancedRandomPointInBox(const FStevesBalancedRandomStream& Stream, const FVector& Min, const FVector& Max) { return Stream.RandPointInBox(FBox(Min, Max)); }

	
	/**
	 * Let the content streaming system know that there is a viewpoint other than a possessed camera that should be taken
	 * into account when deciding what to stream in. This can be useful when you're using a scene capture component,
	 * which if it's capturing a scene that isn't close to a player, can result in blurry textures.
	 * @param ViewOrigin The world space view point
	 * @param ScreenWidth The width in pixels of the screen being rendered
	 * @param FOV Horizontal field of view, in degrees
	 * @param BoostFactor How much to boost the LOD by (1 being normal, higher being higher detail)
	 * @param bOverrideLocation	Whether this is an override location, which forces the streaming system to ignore all other regular locations
	 * @param Duration How long the streaming system should keep checking this location (in seconds). 0 means just for the next Tick.
	 * @param ActorToBoost Optional pointer to an actor who's textures should have their streaming priority boosted
	 */
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Streaming")
	static void AddViewOriginToStreaming(const FVector& ViewOrigin,
	                                     float ScreenWidth,
	                                     float FOV,
	                                     float BoostFactor = 1.0f,
	                                     bool bOverrideLocation = false,
	                                     float Duration = 0.0f,
	                                     AActor* ActorToBoost = nullptr); 
	

	/**
	 * Force content streaming to update. Useful if you need things to stream in sooner than usual.
	 * @param DeltaTime The amount of time to tell the streaming system that has passed.
	 * @param bBlockUntilDone If true, this call will not return until the streaming system has updated
	 */
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|Streaming")
	static void UpdateStreaming(float DeltaTime, bool bBlockUntilDone = false);
	
};
