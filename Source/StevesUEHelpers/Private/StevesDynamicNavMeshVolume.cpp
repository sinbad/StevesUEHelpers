
#include "StevesDynamicNavMeshVolume.h"

#include "NavigationSystem.h"
#include "Components/BrushComponent.h"
#include "PhysicsEngine/BodySetup.h"


AStevesDynamicNavMeshVolume::AStevesDynamicNavMeshVolume()
{

}

void AStevesDynamicNavMeshVolume::SetLocationAndDimensions(const FVector& Location, const FVector& NewDimensions)
{
	SetActorLocation(Location);
	UpdateDimensions(NewDimensions);
	NotifyNavSystem();
}

void AStevesDynamicNavMeshVolume::SetDimensions(const FVector& NewDimensions)
{
	UpdateDimensions(NewDimensions);
	NotifyNavSystem();
}

void AStevesDynamicNavMeshVolume::UpdateDimensions(const FVector& NewDimensions)
{
	// Volumes are built using UCubeBuilder, but we can't use that class at runtime (Editor only)
	// It generates the 6 faces as polys, like old BSP stuff. No idea why for a cube, we don't need that here
	// Just box it baby

	if (auto Body = GetBrushComponent()->GetBodySetup())
	{
		Body->AggGeom.ConvexElems.Empty();
		if (Body->AggGeom.BoxElems.Num() == 0)
		{
			Body->AggGeom.BoxElems.Emplace();
		}
		auto& Box = Body->AggGeom.BoxElems[0];
		Box.X = NewDimensions.X;
		Box.Y = NewDimensions.Y;
		Box.Z = NewDimensions.Z;
		// Bounds are in World Space, hence use actor location as origin
		GetBrushComponent()->Bounds = FBoxSphereBounds(GetActorLocation(), NewDimensions*0.5f, NewDimensions.GetMax()*0.5f);
	}

}

void AStevesDynamicNavMeshVolume::NotifyNavSystem()
{
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->OnNavigationBoundsUpdated(this);
	}
}
