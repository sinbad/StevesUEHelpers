// Copyright 2020 Old Doorways Ltd


#include "StevesEditorVisComponent.h"
#include "StaticMeshResources.h"
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
	for (auto& A : Arrows)
	{
		Ret->ArrowLines.Add(FDebugRenderSceneProxy::FArrowLine(XForm.TransformPosition(A.Start),
														XForm.TransformPosition(A.End), A.Colour));
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
	for (auto& S : Spheres)
	{
		Ret->Spheres.Add(FStevesDebugRenderSceneProxy::FSphere(
			XForm.GetMaximumAxisScale() * S.Radius,
			XForm.TransformPosition(S.Location),
			S.Colour
			));
	}
	for (auto& Box : Boxes)
	{
		FVector HalfSize = Box.Size * 0.5f;
		FBox DBox(-HalfSize, HalfSize);
		// Apply local rotation first then parent transform
		FTransform CombinedXForm = FTransform(Box.Rotation, Box.Location) * XForm;
		Ret->Boxes.Add(FStevesDebugRenderSceneProxy::FDebugBox(
			DBox, Box.Colour, CombinedXForm));
	}
	for (auto& Cylinder : Cylinders)
	{
		// Apply local rotation first then parent transform
		const FTransform CombinedXForm = FTransform(Cylinder.Rotation, Cylinder.Location) * XForm;
		const float HalfH = Cylinder.Height * 0.5f * CombinedXForm.GetScale3D().Z;
		const float R = Cylinder.Radius * CombinedXForm.GetScale3D().Z;
		const FVector Centre = CombinedXForm.TransformPosition(FVector::ZeroVector);
		const FVector LocalX = CombinedXForm.TransformVectorNoScale(FVector::ForwardVector);
		const FVector LocalY = CombinedXForm.TransformVectorNoScale(FVector::RightVector);
		const FVector LocalZ = CombinedXForm.TransformVectorNoScale(FVector::UpVector);
		Ret->CylindersImproved.Add(FStevesDebugRenderSceneProxy::FDebugCylinder(
			Centre, LocalX, LocalY, LocalZ, R, HalfH, 16, Cylinder.Colour));
	}
	for (auto& Capsule : Capsules)
	{
		// Apply local rotation first then parent transform
		const FTransform CombinedXForm = FTransform(Capsule.Rotation, Capsule.Location) * XForm;
		const float HalfH = Capsule.Height * 0.5f * CombinedXForm.GetScale3D().Z;
		const float R = Capsule.Radius * CombinedXForm.GetScale3D().Z;
		const FVector Position = CombinedXForm.TransformPosition(FVector::ZeroVector);
		const FVector LocalX = CombinedXForm.TransformVectorNoScale(FVector::ForwardVector);
		const FVector LocalY = CombinedXForm.TransformVectorNoScale(FVector::RightVector);
		const FVector LocalZ = CombinedXForm.TransformVectorNoScale(FVector::UpVector);
		Ret->CapsulesImproved.Add(FStevesDebugRenderSceneProxy::FCapsule(
			Position, R,
			LocalX, LocalY, LocalZ,
			HalfH, Capsule.Colour));
	}
	for (auto& Mesh : Meshes)
	{
		// Apply local rotation first then parent transform
		if (IsValid(Mesh.Mesh))
		{
			const FTransform CombinedXForm = FTransform(Mesh.Rotation, Mesh.Location, Mesh.Scale) * XForm;
			const FStaticMeshLODResources& Lod = Mesh.Mesh->GetLODForExport(Mesh.bUseLowestLOD ? Mesh.Mesh->GetNumLODs() - 1 : 0);
			TArray<FDynamicMeshVertex> Vertices;
			TArray<uint32> Indices;

			Lod.IndexBuffer.GetCopy(Indices);
			auto& PosBuffer = Lod.VertexBuffers.PositionVertexBuffer;
			uint32 NumVerts = PosBuffer.GetNumVertices();
			Vertices.Reserve(NumVerts);
			for (uint32 i = 0; i < NumVerts; ++i)
			{
				Vertices.Add(FDynamicMeshVertex(PosBuffer.VertexPosition(i)));
			}
			
			Ret->MeshesImproved.Add(FStevesDebugRenderSceneProxy::FDebugMesh(CombinedXForm.ToMatrixWithScale(), Vertices, Indices, Mesh.Colour));
			
		}
		
		
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
	for (auto& A : Arrows)
	{
		// Re-centre the origin of the line to make box extents 
		FVector Extents = A.Start.GetAbs().ComponentMax(A.End.GetAbs());
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
	for (auto& S : Spheres)
	{
		B = B + FBoxSphereBounds(S.Location, FVector(S.Radius), S.Radius);
	}
	for (auto& Box : Boxes)
	{
		FVector HalfSize = Box.Size * 0.5f;
		FBox DBox(-HalfSize, HalfSize);
		// Apply local rotation only, world is done later
		FTransform BoxXForm = FTransform(Box.Rotation, Box.Location);
		DBox = DBox.TransformBy(BoxXForm);
		B = B + FBoxSphereBounds(DBox);
	}
	for (auto& Cylinder : Cylinders)
	{
		FVector HalfSize = FVector(Cylinder.Radius, Cylinder.Radius, Cylinder.Height * 0.5f);
		FBox DBox(-HalfSize, HalfSize);
		// Apply local rotation only, world is done later
		FTransform XForm = FTransform(Cylinder.Rotation, Cylinder.Location);
		DBox = DBox.TransformBy(XForm);
		B = B + FBoxSphereBounds(DBox);
	}
	for (auto& Capsule : Capsules)
	{
		FVector HalfSize = FVector(Capsule.Radius, Capsule.Radius, Capsule.Height * 0.5f + Capsule.Radius * 2.f);
		FBox DBox(-HalfSize, HalfSize);
		// Apply local rotation only, world is done later
		FTransform XForm = FTransform(Capsule.Rotation, Capsule.Location);
		DBox = DBox.TransformBy(XForm);
		B = B + FBoxSphereBounds(DBox);
	}
	for (auto& Mesh : Meshes)
	{
		if (IsValid(Mesh.Mesh))
		{
			const FTransform XForm = FTransform(Mesh.Rotation, Mesh.Location, Mesh.Scale);
			B = B + Mesh.Mesh->GetBounds().TransformBy(XForm);
		}
		
	}
	return B.TransformBy(LocalToWorld);
}

