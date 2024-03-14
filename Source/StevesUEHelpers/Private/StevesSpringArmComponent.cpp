
#include "StevesSpringArmComponent.h"

#include "StevesUEHelpers.h"

// Set this to 1 to VLOG where camera is being occluded from
#define ENABLE_VLOG_CAMERA_OCCLUSION 0

UStevesSpringArmComponent::UStevesSpringArmComponent()
{
	
}

FVector UStevesSpringArmComponent::BlendLocations(const FVector& DesiredArmLocation,
	const FVector& TraceHitLocation,
	bool bHitSomething,
	float DeltaTime)
{

#if ENABLE_VLOG_CAMERA_OCCLUSION
#if ENABLE_VISUAL_LOG
	if (bHitSomething)
	{
		FVector NewDesiredLoc = PreviousDesiredLoc - PreviousDesiredRot.Vector() * TargetArmLength;
		// Add socket offset in local space
		NewDesiredLoc += FRotationMatrix(PreviousDesiredRot).TransformVector(SocketOffset);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
		FHitResult Result;
		if (GetWorld()->SweepSingleByChannel(Result, PreviousArmOrigin, NewDesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams))
		{
			UE_VLOG_ARROW(this, LogStevesUEHelpers, Log, PreviousArmOrigin, Result.Location, FColor::Cyan, TEXT(""));
			UE_VLOG_LOCATION(this, LogStevesUEHelpers, Log, Result.Location, ProbeSize, FColor::Cyan, TEXT("%s"), *Result.GetActor()->GetActorNameOrLabel());
		}
	}
#endif
#endif
	// These locations are in world space, we only want to blend the arm length, not the rest
	const FVector Base = Super::BlendLocations(DesiredArmLocation, TraceHitLocation, bHitSomething, DeltaTime);

	if (bEnableSmoothCollisionAvoidance)
	{
		// Convert these back to arm lengths
		// PreviousArmOrigin has been already set and is the world space origin
		const float TargetArmLen = (Base - PreviousArmOrigin).Length();
	
		const float NewArmLen = FMath::FInterpTo(PrevArmLength.Get(TargetArmLen), TargetArmLen, DeltaTime, SmoothCollisionAvoidanceSpeed);

		// Perform the same transformation again using the new arm length
		// Now offset camera position back along our rotation
		FVector Ret = PreviousDesiredLoc - PreviousDesiredRot.Vector() * NewArmLen;
		// Add socket offset in local space
		Ret += FRotationMatrix(PreviousDesiredRot).TransformVector(SocketOffset);

		PrevArmLength = NewArmLen;

		return Ret;
	}
	else
	{
		return Base;
	}
	
}
