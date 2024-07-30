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