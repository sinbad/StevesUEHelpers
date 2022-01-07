#pragma once

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
};