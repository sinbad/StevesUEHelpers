#pragma once
#include <functional>

struct FKConvexElem;

/// Helper maths routines that UE4 is missing, all static
class STEVESUEHELPERS_API StevesMathHelpers
{
public:

	/**
	* @brief Return whether a sphere overlaps a cone
	* @param ConeOrigin Origin of the cone
	* @param ConeDir Direction of the cone, must be normalised
	* @param ConeHalfAngle Half-angle of the cone, in radians
	* @param Distance Length of the cone
	* @param SphereCentre Centre of the sphere
	* @param SphereRadius Radius of the sphere
	* @return True if the sphere overlaps the cone
	*/
	static bool SphereOverlapCone(const FVector& ConeOrigin, const FVector& ConeDir, float ConeHalfAngle, float Distance, const FVector& SphereCentre, float SphereRadius)
	{
		// Algorithm from https://www.geometrictools.com/GTE/Mathematics/IntrSphere3Cone3.h
		
		const float SinHalfAngle = FMath::Sin(ConeHalfAngle);
		const float InvSinHalfAngle =  1.f/SinHalfAngle;

		const FVector U = ConeOrigin - (SphereRadius * InvSinHalfAngle) * ConeDir;
		const FVector CmU = SphereCentre - U;
		const float AdCmU = FVector::DotProduct(ConeDir, CmU);
		if (AdCmU > 0)
		{
			const float CosHalfAngle = FMath::Cos(ConeHalfAngle);
			const float CosHalfAngleSq = CosHalfAngle * CosHalfAngle;

			const float sqrLengthCmU = FVector::DotProduct(CmU, CmU);
			if (AdCmU * AdCmU >= sqrLengthCmU * CosHalfAngleSq)
			{
				const FVector CmV = SphereCentre - ConeOrigin;
				const float AdCmV = FVector::DotProduct(ConeDir, CmV);
				if (AdCmV < -SphereRadius)
				{
					return false;
				}

				if (AdCmV > Distance + SphereRadius)
				{
					return false;
				}

				const float rSinAngle = SphereRadius * SinHalfAngle;
				if (AdCmV >= -rSinAngle)
				{
					if (AdCmV <= Distance - rSinAngle)
					{
						return true;
					}
					else
					{
						const float TanHalfAngle = FMath::Tan(ConeHalfAngle);

						const FVector barD = CmV - Distance * ConeDir;
						const float lengthAxBarD = FVector::CrossProduct(ConeDir, barD).Size();
						const float hmaxTanAngle = Distance * TanHalfAngle;
						if (lengthAxBarD <= hmaxTanAngle)
						{
							return true;
						}

						const float AdBarD = AdCmV - Distance;
						const float diff = lengthAxBarD - hmaxTanAngle;
						const float sqrLengthCmBarK = AdBarD * AdBarD + diff * diff;
						return sqrLengthCmBarK <= SphereRadius * SphereRadius;
					}
				}
				else
				{
					const float sqrLengthCmV = FVector::DotProduct(CmV, CmV);
					return sqrLengthCmV <= SphereRadius * SphereRadius;
				}
			}
		}
		return false;
	}

	/**
	 * Explicitly test the overlap of any collision shape with a convex element.
	 * @param Convex The convex element
	 * @param ConvexTransform The world transform of the convex element
	 * @param Shape The test shape
	 * @param ShapePos The test shape world position 
	 * @param ShapeRot The test shape world rotation
	 * @param OutResult Details of the result if returning true
	 * @return Whether this shape overlaps the convex element
	 */
	static bool OverlapConvex(const FKConvexElem& Convex,
					   const FTransform& ConvexTransform,
					   const FCollisionShape& Shape,
					   const FVector& ShapePos,
					   const FQuat& ShapeRot,
					   FMTDResult& OutResult);

	/**
	 * Return the distance to a convex polygon in 2D where points are in the same space
	 * @param ConvexPoints Points on the convex polygon, anti-clockwise order, in a chosen space
	 * @param LocalPoint Point to test, in same space as convex points
	 * @return The distance to this convex polygon in 2D space. <= 0 if inside
	 */
	static float GetDistanceToConvex2D(const TArray<FVector2f>& ConvexPoints,
									   const FVector& LocalPoint);
	/**
	 * Return the distance to a convex polygon in 2D world space, converting between spaces
	 * @param ConvexPoints Points on the convex polygon, anti-clockwise order, in local space
	 * @param ConvexTransform World transform for convex polygon
	 * @param WorldPoint Point in world space
	 * @return The distance to this convex polygon in 2D space. <= 0 if inside
	 */
	static float GetDistanceToConvex2D(const TArray<FVector2f>& ConvexPoints,
	                                   const FTransform& ConvexTransform,
	                                   const FVector& WorldPoint)
	{
		checkf(ConvexTransform.GetMaximumAxisScale() == ConvexTransform.GetMinimumAxisScale(), TEXT("Non-uniform scale not supported in GetDistanceToConvex2D"));
		const FVector LocalPoint = ConvexTransform.InverseTransformPosition(WorldPoint);

		// Need to rescale distance back up to world scale, only uniform scale supported for simplicity
		return GetDistanceToConvex2D(ConvexPoints, LocalPoint) * ConvexTransform.GetScale3D().X;
	}
	
	/**
	 * Returns whether a 2D point is inside a triangle
	 * @param p Point to test
	 * @param v0 First triangle point
	 * @param v1 Second triangle point
	 * @param v2 Third triangle point
	 * @return Whether point p is inside the triangle.
	 */
	static bool IsPointInTriangle2D(const FVector& p,
	                                const FVector2f& v0,
	                                const FVector2f& v1,
	                                const FVector2f& v2)
	{
		const float s = (v0.X - v2.X) * (p.Y - v2.Y) - (v0.Y - v2.Y) * (p.X - v2.X);
		const float t = (v1.X - v0.X) * (p.Y - v0.Y) - (v1.Y - v0.Y) * (p.X - v0.X);

		if ((s < 0) != (t < 0) && s != 0 && t != 0)
			return false;

		const float d = (v2.X - v1.X) * (p.Y - v1.Y) - (v2.Y - v1.Y) * (p.X - v1.X);
		return d == 0 || (d < 0) == (s + t <= 0);
	}

	/**
	 * Function that tries to fill a 2D area with the largest rectangles it can. The area is abstractly defined as a boundary
	 * index area with start X/Y and width/height, and will call back the CellIncludeFunc to determine whether a given
	 * cell index X/Y should be considered valid to include in a rectangle. This means you can define irregular grids of
	 * "valid" cells, and this function will fill the area with the largest rectangles it can while staying out of "invalid"
	 * cells.
	 * If you return "true" from every call to your CellIncludeFunc then the result will be a single rectangle covering
	 * the entire area. It's expected that you will return "false" for some X/Y combinations and that will cause the area
	 * to be split into multiple rectangles.
	 * The returned rectangles will not overlap, and the entire valid area will be filled.
	 * @param StartX The start X index. This is defined by your own data, so you can address a subset if you want.
	 * @param StartY The start Y index.This is defined by your own data, so you can address a subset if you want.
	 * @param Width The width of the area to fill. This is defined by your own data, so you can address a subset if you want.
	 * @param Height The height of the area to fill. This is defined by your own data, so you can address a subset if you want.
	 * @param CellIncludeFunc Your function which given an X/Y cell index, must return true if that cell is valid to be
	 *		included in a rectangle.
	 * @param OutRects Array of rectangles which this function should append results to. Will not be cleared before adding.
	 * @return The number of rectangles added by this call. Each rectangle is a min/max inclusive X/Y value.
	 */
	static int Fill2DRegionWithRectangles(int StartX,
	                                      int StartY,
	                                      int Width,
	                                      int Height,
	                                      std::function<bool(int, int)> CellIncludeFunc,
	                                      TArray<FIntRect>& OutRects);

	
};