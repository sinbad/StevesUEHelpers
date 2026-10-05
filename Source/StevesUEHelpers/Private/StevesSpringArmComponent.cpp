// Copyright Steve Streeting 2020 onwards
// Released under the MIT license

#include "StevesSpringArmComponent.h"
#include "VisualLogger/VisualLogger.h"
#include "StevesUEHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"


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
	
	
	// Base spring arm doesn't interpolate at all unless tracing is enabled, but it's far more useful
	// to interpolate in all cases even if we're changing it manually
	// So instead of calling super, we'll copy & change the implementation
	
	// -- Start of Super::UpdateDesiredArmLocation
	FRotator DesiredRot = GetTargetRotation();

	// If our viewtarget is simulating using physics, we may need to clamp deltatime
	if (bClampToMaxPhysicsDeltaTime)
	{
		// Use the same max timestep cap as the physics system to avoid camera jitter when the viewtarget simulates less time than the camera
		DeltaTime = FMath::Min(DeltaTime, UPhysicsSettings::Get()->MaxPhysicsDeltaTime);
	}

	// Apply 'lag' to rotation if desired
	if(bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}		

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin,  7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	// Here's where we change things, blend even when trace not enabled
	if (/* bDoTrace  && */ (TargetArmLength != 0.0f))
	{
		bIsCameraFixed = true;
		
		FHitResult Result;
		if (bDoTrace)
		{
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());
			GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);
		}
		else
		{
			Result.Location = DesiredLoc;
			Result.bBlockingHit = false;
		}
		
		UnfixedCameraPosition = DesiredLoc;
		ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc) 
		{	
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();

	// -- End of Super::UpdateDesiredArmLocation

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

void UStevesSpringArmComponent::JumpToDesiredLocation()
{
	CancelTargetOffsetSmooth();
	CancelSocketOffsetSmooth();
	UpdateDesiredArmLocation(false, false, false, 0);
}
