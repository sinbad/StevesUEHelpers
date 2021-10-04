#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

typedef TSharedPtr<struct FStevesTextureRenderTargetReservation> FStevesTextureRenderTargetReservationPtr;
typedef TSharedPtr<struct FStevesTextureRenderTargetPool> FStevesTextureRenderTargetPoolPtr;

/// Holder for an assigned texture. While this structure exists, the texture will be considered assigned 
/// and will not be returned from any other request. Once this structure is destroyed the texture will
/// be free for re-use. For that reason, only pass this structure around by SharedRef/SharedPtr.
/// The texture is held by a weak pointer however, the strong pointer is held by the pool. The texture will continue
/// to be available to this reservation except if the pool is told to forcibly release textures.
struct STEVESUEHELPERS_API FStevesTextureRenderTargetReservation
{
public:
	/// The texture. May be null if the pool has forcibly reclaimed the texture prematurely
	TWeakObjectPtr<UTextureRenderTarget2D> Texture;
	TWeakPtr<struct FStevesTextureRenderTargetPool> Owner;

	FStevesTextureRenderTargetReservation() = default;

	FStevesTextureRenderTargetReservation(UTextureRenderTarget2D* InTexture,
	                                 FStevesTextureRenderTargetPoolPtr InOwner)
		: Texture(InTexture),
		  Owner(InOwner)
	{
	}
	
	~FStevesTextureRenderTargetReservation();
};


/**
 * A pool of render target textures. To save pre-creating render textures as assets, and to control the re-use of
 * these textures at runtime.
 */
struct STEVESUEHELPERS_API FStevesTextureRenderTargetPool : public TSharedFromThis<FStevesTextureRenderTargetPool>
{

protected:
	/// The name of the pool. It's possible to have more than one texture pool.
	FName PoolName;

	struct FTextureKey
	{
		FIntPoint Size;
		ETextureRenderTargetFormat Format;

		friend bool operator==(const FTextureKey& Lhs, const FTextureKey& RHS)
		{
			return Lhs.Size == RHS.Size
				&& Lhs.Format == RHS.Format;
		}

		friend bool operator!=(const FTextureKey& Lhs, const FTextureKey& RHS)
		{
			return !(Lhs == RHS);
		}

		friend uint32 GetTypeHash(const FTextureKey& Key)
		{
			return HashCombine(GetTypeHash(Key.Size), static_cast<uint32>(Key.Format));
		}

	};

	TMultiMap<FTextureKey, TSharedPtr<UTextureRenderTarget2D>> UnreservedTextures;
	TArray<TSharedPtr<UTextureRenderTarget2D>> ReservedTextures;
	

	friend struct FStevesTextureRenderTargetReservation;
	/// Release a reservation on a texture, allowing it back into the pool
	/// Protected because only FStevesTextureRenderTargetReservation will need to do this.
	void ReleaseTextureReservation(UTextureRenderTarget2D* Tex);	

public:
	FStevesTextureRenderTargetPool() = default;

	explicit FStevesTextureRenderTargetPool(const FName& PoolName)
		: PoolName(PoolName)
	{
	}

	const FName& GetPoolName() const { return PoolName; }

	/**
	 * Reserve a texture for use as a render target. This will create a new texture target if needed. 
	 * @param Size The dimensions of the texture
	 * @param Format Format of the texture
	 * @return A shared pointer to a structure which holds the reservation for this texture. When that structure is
	 * destroyed, it will release the texture back to the pool.
	 */
	FStevesTextureRenderTargetReservationPtr ReserveTexture(FIntPoint Size, ETextureRenderTargetFormat Format);

	/**
	 * Forcibly revoke all reservations in this pool. Reservations which have been made will have their weak texture
	 * pointers invalidated.
	 */
	void RevokeAllReservations();
	
	/**
	 * Destroy previously created textures and free the memory.
	 * @param bRevokeReservations If false, only destroys unreserved textures. If true, destroys reserved textures
	 * as well (the weak pointer on their reservations will cease to be valid)
	 */
	void DrainPool(bool bRevokeReservations = false);
	
};

