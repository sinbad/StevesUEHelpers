
#include "StevesSpringArmComponent.h"


UStevesSpringArmComponent::UStevesSpringArmComponent()
{
	
}

FVector UStevesSpringArmComponent::BlendLocations(const FVector& DesiredArmLocation,
	const FVector& TraceHitLocation,
	bool bHitSomething,
	float DeltaTime)
{
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
