#include "StevesMathHelpers.h"

#include "Chaos/CastingUtilities.h"
#include "Chaos/GeometryQueries.h"

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
