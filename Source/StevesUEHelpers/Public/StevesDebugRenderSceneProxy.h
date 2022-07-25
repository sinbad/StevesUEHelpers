// Copyright 2020 Old Doorways Ltd

#pragma once

#include "CoreMinimal.h"
#include "DebugRenderSceneProxy.h"

/**
 * An extension to FDebugRenderSceneProxy to support other shapes, e.g. circles and arcs
 */
class FStevesDebugRenderSceneProxy : public FDebugRenderSceneProxy
{
public:
	FStevesDebugRenderSceneProxy(const UPrimitiveComponent* InComponent)
		: FDebugRenderSceneProxy(InComponent)
	{
	}

	STEVESUEHELPERS_API virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
	                                    uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	STEVESUEHELPERS_API virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	struct FDebugCircle
	{
		FDebugCircle(const FVector& InCentre, const FVector& InX, const FVector& InY, float InRadius, int InNumSegments,
		             const FColor& InColor, float InThickness = 0) :
			Centre(InCentre),
			X(InX),
			Y(InY),
			Radius(InRadius),
			NumSegments(InNumSegments),
			Color(InColor),
			Thickness(InThickness)
		{
		}

		FVector Centre;
		FVector X;
		FVector Y;
		float Radius;
		int NumSegments;
		FColor Color;
		float Thickness;
	};

	/// An arc which is a section of a circle
	struct FDebugArc
	{
		FDebugArc(const FVector& InCentre, const FVector& InX, const FVector& InY, float InMinAngle, float InMaxAngle,
		          float InRadius, int InNumSegments, const FColor& InColor) :
			Centre(InCentre),
			X(InX),
			Y(InY),
			MinAngle(InMinAngle),
			MaxAngle(InMaxAngle),
			Radius(InRadius),
			NumSegments(InNumSegments),
			Color(InColor)
		{
		}

		FVector Centre;
		FVector X;
		FVector Y;
		float MinAngle;
		float MaxAngle;
		float Radius;
		int NumSegments;
		FColor Color;
	};

	/// Replacement for FWireCylinder because that's garbage and doesn't reflect the component transform
	struct FDebugCylinder
	{
		FDebugCylinder(const FVector &InCentre, const FVector& InX, const FVector& InY, const FVector& InZ, const float InRadius, const float InHalfHeight, int InNumSegments, const FColor &InColor) :
			Centre(InCentre),
			X(InX), Y(InY), Z(InZ),
			Radius(InRadius),
			HalfHeight(InHalfHeight),
			NumSegments(InNumSegments),
			Color(InColor)
		{
		}

		FVector Centre;
		FVector X;
		FVector Y;
		FVector Z;
		float Radius;
		float HalfHeight;
		int NumSegments;
		FColor Color;
	};

	struct FDebugMesh
	{
		FDebugMesh(const FMatrix& InLocalToWorld,
			const TArray<FDynamicMeshVertex>& InVertices,
			const TArray<uint32>& InIndices,
			const FColor& InColor)
			: LocalToWorld(InLocalToWorld),
			  Vertices(InVertices),
			  Indices(InIndices),
			  Color(InColor)
		{
		}

		FMatrix LocalToWorld;
		TArray<FDynamicMeshVertex>	Vertices;
		TArray <uint32> Indices;
		FColor Color;
	};

	TArray<FDebugCircle> Circles;
	TArray<FDebugArc> Arcs;
	TArray<FDebugCylinder> CylindersImproved; // Because we need our own
	TArray<FCapsule> CapsulesImproved; // Because we need our own
	TArray<FDebugMesh> MeshesImproved; // Because we need our own

};
