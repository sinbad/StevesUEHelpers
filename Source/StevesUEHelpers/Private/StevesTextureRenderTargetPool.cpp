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
	const FTextureKey Key {Size, Format};
	UTextureRenderTarget2D* Tex = nullptr;
	if (auto Pooled = UnreservedTextures.Find(Key))
	{
		Tex = *Pooled;
		UnreservedTextures.RemoveSingle(Key, Tex);
	}
	else if (Size.X > 0 && Size.Y > 0)
	{
		// No existing texture, so create
		// Texture owner should be a valid UObject that will determine lifespan
		UObject* TextureOwner = PoolOwner.IsValid() ? PoolOwner.Get() : GetTransientPackage();
		Tex = NewObject<UTextureRenderTarget2D>(TextureOwner);
		Tex->RenderTargetFormat = Format;
		Tex->InitAutoFormat(Size.X, Size.Y);
		Tex->UpdateResourceImmediate(true);
	}

	// Record reservation
	Reservations.Add(FReservationInfo(Key, Owner, Tex));
	
	return MakeShared<FStevesTextureRenderTargetReservation>(Tex, this->AsShared(), Owner);
}

void FStevesTextureRenderTargetPool::RevokeReservations(const UObject* ForOwner)
{
	// TODO
}

void FStevesTextureRenderTargetPool::DrainPool(bool bForceAndRevokeReservations)
{
	// TODO
}
