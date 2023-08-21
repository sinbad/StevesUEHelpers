// Copyright 2020 Old Doorways Ltd


#include "StevesDebugRenderSceneProxy.h"

// Added in UE 5.2
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
#include "Materials/MaterialRenderProxy.h"
#endif


void FStevesDebugRenderSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                                          const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	FDebugRenderSceneProxy::GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			// Draw Circles
			for (const auto& C : Circles)
			{
				DrawCircle(PDI, C.Centre, C.X, C.Y, C.Color, C.Radius, C.NumSegments, SDPG_World, C.Thickness, 0, C.Thickness > 0);
			}

			// Draw Arcs
			for (const auto& C : Arcs)
			{
				::DrawArc(PDI, 
					C.Centre, 
					C.X, C.Y, 
					C.MinAngle, C.MaxAngle,
					C.Radius, C.NumSegments,
					C.Color, SDPG_Foreground);	
			}
			// Draw Cylinders (properly! superclass ignores transforms)
			for (const auto& C : CylindersImproved)
			{
				::DrawWireCylinder(PDI,
				                   C.Centre,
				                   C.X,
				                   C.Y,
				                   C.Z,
				                   C.Color,
				                   C.Radius,
				                   C.HalfHeight,
				                   C.NumSegments,
				                   SDPG_Foreground);
			}
			for (const auto& C : CapsulesImproved)
			{
				::DrawWireCapsule(PDI,
#if ENGINE_MAJOR_VERSION >= 5					
								   C.Base,
#else
								   C.Location,
#endif
								   C.X,
								   C.Y,
								   C.Z,
								   C.Color,
								   C.Radius,
								   C.HalfHeight,
								   16,
								   SDPG_Foreground);
			}

			
			for (const auto& Mesh : MeshesImproved)
			{
				FDynamicMeshBuilder MeshBuilder(View->GetFeatureLevel());
				MeshBuilder.AddVertices(Mesh.Vertices);
				MeshBuilder.AddTriangles(Mesh.Indices);

				// Parent caches these (only within this function) but let's assume that's not worth it. Will people really
				// have lots of meshes with a shared colour in this single context to make it worth it?
				const auto MatRenderProxy = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial->GetRenderProxy(), Mesh.Color);
				FDynamicMeshBuilderSettings Settings;
				Settings.bWireframe = true;
				Settings.bUseSelectionOutline = false;
				Settings.bUseWireframeSelectionColoring = true;
				MeshBuilder.GetMesh(Mesh.LocalToWorld, MatRenderProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector);
			}
			
		}
	}
}

FPrimitiveViewRelevance FStevesDebugRenderSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	// More useful defaults than FDebugRenderSceneProxy
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bDynamicRelevance = true;
	Result.bShadowRelevance = false;
	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	return Result;
}
