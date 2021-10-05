#include "StevesTextureRenderTargetPool.h"

FStevesTextureRenderTargetReservation::~FStevesTextureRenderTargetReservation()
{
	UE_LOG(LogTemp, Warning, TEXT("Releasing texture reservation"));
	if (ParentPool.IsValid() && Texture.IsValid())
	{
		ParentPool.Pin()->ReleaseTextureReservation(Texture.Get());
		Texture = nullptr;
	}
}

void FStevesTextureRenderTargetPool::ReleaseTextureReservation(UTextureRenderTarget2D* Tex)
{
	// TODO
}

FStevesTextureRenderTargetReservationPtr FStevesTextureRenderTargetPool::ReserveTexture(FIntPoint Size,
	ETextureRenderTargetFormat Format, const UObject* Owner)
{
	// TODO
	return MakeShared<FStevesTextureRenderTargetReservation>(nullptr, this->AsShared());
}

void FStevesTextureRenderTargetPool::RevokeReservations(const UObject* ForOwner)
{
	// TODO
}

void FStevesTextureRenderTargetPool::DrainPool(bool bForceAndRevokeReservations)
{
	// TODO
}
