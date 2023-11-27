#include "StevesMathHelpers.h"

#include "Chaos/CastingUtilities.h"
#include "Chaos/GeometryQueries.h"
#include "PhysicsEngine/ConvexElem.h"

bool StevesMathHelpers::OverlapConvex(const FKConvexElem& Convex,
                                      const FTransform& ConvexTransform,
                                      const FCollisionShape& Shape,
                                      const FVector& ShapePos,
                                      const FQuat& ShapeRot,
                                      FMTDResult& OutResult)
{
	const FPhysicsShapeAdapter ShapeAdapter(ShapeRot, Shape);
	const TSharedPtr<Chaos::FConvex, ESPMode::ThreadSafe>& ChaosConvex = Convex.GetChaosConvexMesh();
	if (!ChaosConvex.IsValid())
		return false;

	const Chaos::FImplicitObject& ShapeGeom = ShapeAdapter.GetGeometry();
	const FTransform& ShapeGeomTransform = ShapeAdapter.GetGeomPose(ShapePos);

	OutResult.Distance = 0;

	bool bHasOverlap = false;

	Chaos::FMTDInfo MTDInfo;
	if (Chaos::Utilities::CastHelper(ShapeGeom,
	                                 ShapeGeomTransform,
	                                 [&](const auto& Downcast, const auto& FullGeomTransform)
	                                 {
		                                 return Chaos::OverlapQuery(*ChaosConvex.Get(),
		                                                            ConvexTransform,
		                                                            Downcast,
		                                                            FullGeomTransform,
		                                                            /*Thickness=*/
		                                                            0,
		                                                            &MTDInfo);
	                                 }))
	{
		bHasOverlap = true;
		OutResult.Distance = MTDInfo.Penetration;
		OutResult.Direction = MTDInfo.Normal;
	}

	return bHasOverlap;	
}

float StevesMathHelpers::GetDistanceToConvex2D(const TArray<FVector2f>& ConvexPoints, const FVector& LocalPoint)
{
	// Assume inside until 1 or more tests show it's outside
	bool bInside = true;
	float ClosestOutside = 1e30f;
	float ClosestInside = 1e30f;
	const int N = ConvexPoints.Num();
	for (int i = 0; i < N; ++i)
	{
		const int OtherIdx = (i + 1) % N;
		const FVector2f& Start = ConvexPoints[i];
		const FVector2f& End = ConvexPoints[OtherIdx];
		const FVector2f Line = End - Start;
		const FVector2f P = FVector2f(LocalPoint.X, LocalPoint.Y);
		// Determine inside / outside first
		// This may be the distance, but might not be if perpendicular projection is outside line segment
		// So save the normalise for later
		// Simple cross product to get the (non unit length) normal
		const FVector2f Normal = FVector2f(-Line.Y, Line.X);
		const FVector2f ToPoint = P - ConvexPoints[i];
		const float DotNormal = Normal.Dot(ToPoint);

		if (DotNormal > 0)
		{
			// If >0 result is outside, point must be outside
			bInside = false;
		}
			
		// Do a perpendicular projection onto the line segment to see if we're within the limits of it
		const float DotLine = Line.Dot(ToPoint);
		const float T = DotLine / Line.SquaredLength();

		float Dist;
		if (T < 0)
		{
			// Outside line segment, closest point is start
			Dist = (P - Start).Length();
		}
		else if (T > 1)
		{
			// Outside line segment, closest point is end
			Dist = (P - End).Length();
		}
		else
		{
			// Within line segment
			Dist = FMath::Sqrt(ToPoint.SquaredLength() - FMath::Square(T * Line.Length()));
		}

		if (DotNormal > 0)
		{
			ClosestOutside = FMath::Min(ClosestOutside, Dist);
		}
		else
		{
			ClosestInside = FMath::Min(ClosestInside, Dist);
		}
	}

	return bInside ? -ClosestInside : ClosestOutside;		
}
