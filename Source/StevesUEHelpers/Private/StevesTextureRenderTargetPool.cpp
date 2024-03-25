#include "StevesTextureRenderTargetPool.h"

#include "StevesUEHelpers.h"
#include "Kismet/KismetRenderingLibrary.h"

FStevesTextureRenderTargetReservation::~FStevesTextureRenderTargetReservation()
{
	//UE_LOG(LogStevesUEHelpers, Log, TEXT("FStevesTextureRenderTargetReservation: destruction"));
	if (ParentPool.IsValid() && Texture.IsValid())
	{
		ParentPool.Pin()->ReleaseReservation(Texture.Get());
		Texture = nullptr;
	}
}

void FStevesTextureRenderTargetPool::ReleaseReservation(UTextureRenderTarget2D* Tex)
{
	if (!Tex)
	{
		UE_LOG(LogStevesUEHelpers, Warning, TEXT("FStevesTextureRenderTargetPool: Attempted to release a null texture"));
		return;
	}
	
	for (int i = 0; i < Reservations.Num(); ++i)
	{
		const FReservationInfo& R = Reservations[i];
		if (R.Texture.IsValid() && R.Texture.Get() == Tex)
		{
			UE_LOG(LogStevesUEHelpers, Verbose, TEXT("FStevesTextureRenderTargetPool: Released texture reservation on %s"), *Tex->GetName());
			UnreservedTextures.Add(R.Key, Tex);
			Reservations.RemoveAtSwap(i);
			ReservedTextures.Remove(Tex);
			
			return;
		}
	}

	UE_LOG(LogStevesUEHelpers, Warning, TEXT("FStevesTextureRenderTargetPool: Attempted to release a reservation on %s that was not found"), *Tex->GetName());

}

FStevesTextureRenderTargetPool::~FStevesTextureRenderTargetPool()
{
	DrainPool(true);
}

void FStevesTextureRenderTargetPool::AddReferencedObjects(FReferenceCollector& Collector)
{
	// We need to hold on to the texture references
	Collector.AddReferencedObjects(ReservedTextures);
	Collector.AddReferencedObjects(UnreservedTextures);
}

#if ENGINE_MAJOR_VERSION >= 5
FString FStevesTextureRenderTargetPool::GetReferencerName() const
{
	return "FStevesTextureRenderTargetPool";
}
#endif

FStevesTextureRenderTargetReservationPtr FStevesTextureRenderTargetPool::ReserveTexture(FIntPoint Size,
                                                                                        ETextureRenderTargetFormat Format, const UObject* Owner)
{
	const FTextureKey Key {Size, Format};
	UTextureRenderTarget2D* Tex = nullptr;
	if (auto Pooled = UnreservedTextures.Find(Key))
	{
		Tex = *Pooled;
		UnreservedTextures.RemoveSingle(Key, Tex);
		UE_LOG(LogStevesUEHelpers, Verbose, TEXT("FStevesTextureRenderTargetPool: Re-used pooled texture %s"), *Tex->GetName());
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

		UE_LOG(LogStevesUEHelpers, Verbose, TEXT("FStevesTextureRenderTargetPool: Created new texture %s"), *Tex->GetName());
	}

	// Record reservation
	Reservations.Add(FReservationInfo(Key, Owner, Tex));

	// Reservation doesn't keep the texture alive; if caller doesn't hold a strong pointer to it, it'll be destroyed
	// So we need to hold it ourselves
	ReservedTextures.Add(Tex);
	
	return MakeShared<FStevesTextureRenderTargetReservation>(Tex, this->AsShared(), Owner);
}

void FStevesTextureRenderTargetPool::RevokeReservations(const UObject* ForOwner)
{
	for (int i = 0; i < Reservations.Num(); ++i)
	{
		const FReservationInfo& R = Reservations[i];
		if (!ForOwner || R.Owner == ForOwner)
		{
			if (R.Texture.IsValid())
			{
				UE_LOG(LogStevesUEHelpers, Verbose, TEXT("FStevesTextureRenderTargetPool: Revoked texture reservation on %s"), *R.Texture->GetName());
				UnreservedTextures.Add(R.Key, R.Texture.Get());
				ReservedTextures.Remove(R.Texture.Get());
			}
			// Can't use RemoveAtSwap because it'll change order
			Reservations.RemoveAt(i);
			// Adjust index backwards to compensate
			--i;
		}
	}
}

void FStevesTextureRenderTargetPool::DrainPool(bool bForceAndRevokeReservations)
{
	if (bForceAndRevokeReservations)
		RevokeReservations();

	for (auto& TexPair : UnreservedTextures)
	{
		UKismetRenderingLibrary::ReleaseRenderTarget2D(TexPair.Value);
	}
	UnreservedTextures.Empty();
	ReservedTextures.Empty();

}
