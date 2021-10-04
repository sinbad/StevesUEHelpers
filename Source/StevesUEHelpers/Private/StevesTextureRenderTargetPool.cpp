#include "StevesTextureRenderTargetPool.h"

FStevesTextureRenderTargetReservation::~FStevesTextureRenderTargetReservation()
{
	if (Owner.IsValid() && Texture.IsValid())
	{
		Owner.Pin()->ReleaseTextureReservation(Texture.Get());
		Texture = nullptr;
	}
}

void FStevesTextureRenderTargetPool::ReleaseTextureReservation(UTextureRenderTarget2D* Tex)
{
	// TODO
}

FStevesTextureRenderTargetReservationPtr FStevesTextureRenderTargetPool::ReserveTexture(FIntPoint Size,
	ETextureRenderTargetFormat Format)
{
	// TODO
	return MakeShared<FStevesTextureRenderTargetReservation>(nullptr, this->AsShared());
}

void FStevesTextureRenderTargetPool::RevokeAllReservations()
{
	// TODO
}

void FStevesTextureRenderTargetPool::DrainPool(bool bRevokeReservations)
{
	// TODO
}
