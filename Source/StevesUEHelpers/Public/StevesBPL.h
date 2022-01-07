// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "StevesMathHelpers.h"
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
	 */
	UFUNCTION(BlueprintCallable, Category="StevesUEHelpers|UI")
	static void InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex = 0);
	
};
