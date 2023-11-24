#pragma once

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
									   const FVector& LocalPoint)
	{
		// Assume inside until 1 or more tests show it's outside
		bool bInside = true;
		float ClosestOutside = 0;
		float ClosestInside = -1e30f;
		const int N = ConvexPoints.Num();
		for (int i = 0; i < N; ++i)
		{
			const int OtherIdx = (i + 1) % N;
			const FVector2f Edge = ConvexPoints[OtherIdx] - ConvexPoints[i];
			// Simple cross procduct to get the normal
			const FVector2f Normal = FVector2f(Edge.Y, -Edge.X).GetSafeNormal();
			if (!Normal.IsNearlyZero())
			{
				const FVector2f ToPoint = FVector2f(LocalPoint.X, LocalPoint.Y) - ConvexPoints[i];
				const float Dot = Normal.Dot(ToPoint);

				if (Dot > 0)
				{
					// Outside
					// When outside we want the largest separation because convex
					ClosestOutside = FMath::Max(ClosestOutside, Dot);
					bInside = false;
				}
				else if (bInside)
				{
					// Inside (don't bother if we've already found one outside)
					// Max because negative and inside we want the closest dot
					ClosestInside = FMath::Max(ClosestInside, Dot);
				}
			}
		}

		return bInside ? ClosestInside : ClosestOutside;		
	}
	
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

};