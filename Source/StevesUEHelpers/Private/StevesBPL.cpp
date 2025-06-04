// Fill out your copyright notice in the Description page of Project Settings.


#include "StevesBPL.h"

#include "StevesUiHelpers.h"
#include "StevesUI/StevesUI.h"
#include "ContentStreaming.h"

void UStevesBPL::SetWidgetFocus(UWidget* Widget)
{
	SetWidgetFocusProperly(Widget);
}

UPanelSlot* UStevesBPL::InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex)
{
	return StevesUiHelpers::InsertChildWidgetAt(Parent, Child, AtIndex);
}

FStevesBalancedRandomStream UStevesBPL::MakeBalancedRandomStream(int64 Seed)
{
	return FStevesBalancedRandomStream(Seed);
}

void UStevesBPL::AddViewOriginToStreaming(const FVector& ViewOrigin,
	float ScreenWidth,
	float FOV,
	float BoostFactor,
	bool bOverrideLocation,
	float Duration,
	AActor* ActorToBoost)
{
	IStreamingManager::Get().AddViewInformation(ViewOrigin,
	                                            ScreenWidth,
	                                            ScreenWidth / FMath::Tan(FMath::DegreesToRadians(FOV * 0.5f)),
	                                            BoostFactor,
	                                            bOverrideLocation,
	                                            Duration,
	                                            ActorToBoost);
}

void UStevesBPL::UpdateStreaming(float DeltaTime, bool bBlockUntilDone)
{
	FStreamingManagerCollection& SM = IStreamingManager::Get();
	SM.UpdateResourceStreaming(DeltaTime, true);
	if (bBlockUntilDone)
	{
		SM.BlockTillAllRequestsFinished();
	}
}

float UStevesBPL::GetPerceivedLuminance(const FLinearColor& InColour)
{
	// ITU BT.709
	return
		InColour.R * 0.2126f +
		InColour.G * 0.7152f +
		InColour.B * 0.0722f;
}

float UStevesBPL::GetPerceivedLuminance2(const FLinearColor& InColour)
{
	// ITU BT.601
	return
		InColour.R * 0.299f +
		InColour.G * 0.587f +
		InColour.B * 0.114f;
}

float UStevesBPL::HeadingAngle2D(const FVector2D& Dir)
{
	const FVector2D NormDir = Dir.GetSafeNormal();
	float Angle = FMath::Acos(NormDir.X);

	if(NormDir.Y < 0.0f)
	{
		Angle *= -1.0f;
	}

	return Angle;
}

float UStevesBPL::AngleBetween2D(const FVector2D& DirA, const FVector2D& DirB)
{
	const float HeadingA = HeadingAngle2D(DirA);
	const float HeadingB = HeadingAngle2D(DirB);

	// Get the shortest route
	return FMath::FindDeltaAngleRadians(HeadingA, HeadingB);
}

