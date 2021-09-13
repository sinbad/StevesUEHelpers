// Copyright 2020 Old Doorways Ltd


#include "StevesEditorVisComponent.h"
#include "StevesDebugRenderSceneProxy.h"

void UStevesEditorVisComponent::OnRegister()
{
	Super::OnRegister();
	
	// set up some constants
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCastShadow(false);
	SetIsVisualizationComponent(true);
	SetHiddenInGame(true);
	AlwaysLoadOnClient = false;
	
}

FPrimitiveSceneProxy* UStevesEditorVisComponent::CreateSceneProxy()
{
	auto Ret = new FStevesDebugRenderSceneProxy(this);

	const FTransform& XForm = GetComponentTransform();
	for (auto& C : Circles)
	{
		FQuat WorldRot = XForm.TransformRotation(C.Rotation.Quaternion());
		Ret->Circles.Add(FStevesDebugRenderSceneProxy::FDebugCircle(
			XForm.TransformPosition(C.Location),
			WorldRot.GetForwardVector(), WorldRot.GetRightVector(),
			XForm.GetMaximumAxisScale() * C.Radius,
			C.NumSegments, C.Colour
			));
	}

	return Ret;
	
}

FBoxSphereBounds UStevesEditorVisComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds B = Super::CalcBounds(LocalToWorld);

	// Now we need to merge in all components
	for (auto& C : Circles)
	{
		B = B + FBoxSphereBounds(C.Location, FVector(C.Radius), C.Radius);
	}

	return B;
}

