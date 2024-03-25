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
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	const Chaos::FConvexPtr& ChaosConvex = Convex.GetChaosConvexMesh();
#else
	const TSharedPtr<Chaos::FConvex, ESPMode::ThreadSafe>& ChaosConvex = Convex.GetChaosConvexMesh();
#endif
	if (!ChaosConvex.IsValid())
		return false;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	const auto& ChaosConvexObj = *ChaosConvex.GetReference();
#else
	const auto& ChaosConvexObj = *ChaosConvex.Get();
#endif

	const Chaos::FImplicitObject& ShapeGeom = ShapeAdapter.GetGeometry();
	const FTransform& ShapeGeomTransform = ShapeAdapter.GetGeomPose(ShapePos);

	OutResult.Distance = 0;

	bool bHasOverlap = false;

	Chaos::FMTDInfo MTDInfo;
	if (Chaos::Utilities::CastHelper(ShapeGeom,
	                                 ShapeGeomTransform,
	                                 [&](const auto& Downcast, const auto& FullGeomTransform)
	                                 {
		                                 return Chaos::OverlapQuery(ChaosConvexObj,
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

int StevesMathHelpers::Fill2DRegionWithRectangles(int StartX,
	int StartY,
	int Width,
	int Height,
	std::function<bool(int, int)> CellIncludeFunc,
	TArray<FIntRect>& OutRects)
{
	int RectCount = 0;

	const int Len = Width*Height;
	TArray<bool> bDoneMarkers;
	bDoneMarkers.SetNumUninitialized(Len);
	int CellsTodo = 0;

	int StartN = 0;
	const int EndX = StartX + Width - 1;
	const int EndY = StartY + Height - 1;
	// Initialise done markers based on func
	for (int y = 0; y < Height; ++y)
	{
		for (int x = 0; x < Width; ++x)
		{
			const bool Include = CellIncludeFunc(x+StartX, y+StartY);
			bDoneMarkers[y*Width + x] = !Include;
			if (Include)
			{
				if (++CellsTodo == 1)
				{
					// This is the one we'll start with, might as well calculate it while we're here
					StartN = y*Width + x;
				}
			}
		}		
	}

	while (CellsTodo > 0)
	{
		// Find next starting point, from last one, until not done
		for (; StartN < Len && bDoneMarkers[StartN]; ++StartN) {}

		// Shouldn't happen, but just in case
		if (StartN >= Len)
			break;

		// NOTE: this X/Y is local (based at 0,0 not StartX/StartY, for use with DoneMarkers)
		const int LocalStartX = StartN % Width;
		const int LocalStartY = StartN / Width;

		// We try not to create long & thin rects if we can help it, unlike greedy meshing
		// We're greedy in alternate dims to try to make fatter quads
		bool bCanExtendX = true, bCanExtendY = true;
		bool bExtendingX = true;
		int W = 1;
		int H = 1;

		while (bCanExtendX || bCanExtendY)
		{
			// Try extending right
			if (bExtendingX)
			{
				bool ExtendOK = LocalStartX+W < Width;
				for (int TestY = LocalStartY; ExtendOK && TestY < LocalStartY+H; ++TestY)
				{
					if (bDoneMarkers[TestY*Width + LocalStartX+W])
					{
						// No good
						ExtendOK = false;
					}
				}
				if (ExtendOK)
				{
					++W;
				}
				else
				{
					bCanExtendX = false;
				}
				// Flip extending axis if possible
				if (bCanExtendY)
					bExtendingX = false;
			}
			else
			{
				// Try extending down
				bool ExtendOK = LocalStartY+H < Height;
				for (int TestX = LocalStartX; ExtendOK && TestX < LocalStartX+W; ++TestX)
				{
					if (bDoneMarkers[(LocalStartY+H)*Width + TestX])
					{
						// No good
						ExtendOK = false;
					}
				}
				if (ExtendOK)
				{
					++H;
				}
				else
				{
					bCanExtendY = false;
				}
				// Flip extending axis if possible
				if (bCanExtendX)
					bExtendingX = true;
			}
		}

		// We've calculated the max extension
		for (int y = LocalStartY; y < LocalStartY+H; ++y)
		{
			for (int x = LocalStartX; x < LocalStartX+W; ++x)
			{
				bDoneMarkers[y*Width+x] = true;
				--CellsTodo;
			}
		}
		OutRects.Add(FIntRect(StartX+LocalStartX, StartY+LocalStartY, StartX+LocalStartX+W-1, StartY+LocalStartY+H-1));
		++RectCount;
		
	}

	return RectCount;
	
}
