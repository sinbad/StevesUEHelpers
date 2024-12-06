
#include "StevesSpringArmComponent.h"
#include "VisualLogger/VisualLogger.h"
#include "StevesUEHelpers.h"



UStevesSpringArmComponent::UStevesSpringArmComponent(): bEnableSmoothCollisionAvoidance(1)
{
}

void UStevesSpringArmComponent::UpdateDesiredArmLocation(bool bDoTrace,
	bool bDoLocationLag,
	bool bDoRotationLag,
	float DeltaTime)
{
	// Smooth target & socket offsets
	if (SmoothTargetOffsetTarget.IsSet())
	{
		TargetOffset = FMath::VInterpTo(TargetOffset, SmoothTargetOffsetTarget.GetValue(), DeltaTime, SmoothTargetOffsetSpeed);
		if (TargetOffset.Equals(SmoothTargetOffsetTarget.GetValue()))
		{
			CancelTargetOffsetSmooth();
		}
	}
	if (SmoothSocketOffsetTarget.IsSet())
	{
		SocketOffset = FMath::VInterpTo(SocketOffset, SmoothSocketOffsetTarget.GetValue(), DeltaTime, SmoothSocketOffsetSpeed);
		if (SocketOffset.Equals(SmoothSocketOffsetTarget.GetValue()))
		{
			CancelSocketOffsetSmooth();
		}
	}
	Super::UpdateDesiredArmLocation(bDoTrace, bDoLocationLag, bDoRotationLag, DeltaTime);
}

FVector UStevesSpringArmComponent::BlendLocations(const FVector& DesiredArmLocation,
                                                  const FVector& TraceHitLocation,
                                                  bool bHitSomething,
                                                  float DeltaTime)
{

#if WITH_EDITORONLY_DATA && ENABLE_VISUAL_LOG
	if (bHitSomething && bVisualLogCameraCollision)
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

void UStevesSpringArmComponent::SetTargetOffsetSmooth(const FVector& NewTargetOffset, float Speed)
{
	SmoothTargetOffsetTarget = NewTargetOffset;
	SmoothTargetOffsetSpeed = Speed;
}

void UStevesSpringArmComponent::CancelTargetOffsetSmooth()
{
	SmoothTargetOffsetTarget.Reset();
}

void UStevesSpringArmComponent::SetSocketOffsetSmooth(const FVector& NewSocketOffset, float Speed)
{
	SmoothSocketOffsetTarget = NewSocketOffset;
	SmoothSocketOffsetSpeed = Speed;
}

void UStevesSpringArmComponent::CancelSocketOffsetSmooth()
{
	SmoothSocketOffsetTarget.Reset();
}
