// Copyright 2020 Old Doorways Ltd


#include "StevesEditorVisComponent.h"
#include "StevesDebugRenderSceneProxy.h"

UStevesEditorVisComponent::UStevesEditorVisComponent(const FObjectInitializer& ObjectInitializer)
	: UPrimitiveComponent(ObjectInitializer)
{
	// set up some constants
	PrimaryComponentTick.bCanEverTick = false;
	SetCastShadow(false);
#if WITH_EDITORONLY_DATA
	// Note: this makes this component invisible on level instances, not sure why
	SetIsVisualizationComponent(true);
#endif
	SetHiddenInGame(true);
	bVisibleInReflectionCaptures = false;
	bVisibleInRayTracing = false;
	bVisibleInRealTimeSkyCaptures = false;
	AlwaysLoadOnClient = false;
	bIsEditorOnly = true;
	
}

FPrimitiveSceneProxy* UStevesEditorVisComponent::CreateSceneProxy()
{
	auto Ret = new FStevesDebugRenderSceneProxy(this);

	const FTransform& XForm = GetComponentTransform();
	for (auto& L : Lines)
	{
		Ret->Lines.Add(FDebugRenderSceneProxy::FDebugLine(XForm.TransformPosition(L.Start),
		                                                  XForm.TransformPosition(L.End), L.Colour));
	}
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
	for (auto& Arc : Arcs)
	{
		FQuat WorldRot = XForm.TransformRotation(Arc.Rotation.Quaternion());
		Ret->Arcs.Add(FStevesDebugRenderSceneProxy::FDebugArc(
			XForm.TransformPosition(Arc.Location),
			WorldRot.GetForwardVector(), WorldRot.GetRightVector(),
			Arc.MinAngle, Arc.MaxAngle,
			XForm.GetMaximumAxisScale() * Arc.Radius,
			Arc.NumSegments, Arc.Colour
			));
	}

	return Ret;
	
}

FBoxSphereBounds UStevesEditorVisComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds B = Super::CalcBounds(LocalToWorld);

	// Now we need to merge in all components
	for (auto& L : Lines)
	{
		// Re-centre the origin of the line to make box extents 
		FVector Extents = L.Start.GetAbs().ComponentMax(L.End.GetAbs());
		B = B + FBoxSphereBounds(FVector::ZeroVector, Extents, Extents.GetMax());
	}
	for (auto& C : Circles)
	{
		B = B + FBoxSphereBounds(C.Location, FVector(C.Radius), C.Radius);
	}
	for (auto& Arc : Arcs)
	{
		// Just use the entire circle for simplicity
		B = B + FBoxSphereBounds(Arc.Location, FVector(Arc.Radius), Arc.Radius);
	}

	return B.TransformBy(LocalToWorld);
}

