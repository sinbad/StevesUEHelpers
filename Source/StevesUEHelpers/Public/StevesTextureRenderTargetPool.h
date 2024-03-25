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
	TWeakPtr<struct FStevesTextureRenderTargetPool> ParentPool;
	TWeakObjectPtr<const UObject> CurrentOwner;
	
	FStevesTextureRenderTargetReservation() = default;

	FStevesTextureRenderTargetReservation(UTextureRenderTarget2D* InTexture,
	                                      FStevesTextureRenderTargetPoolPtr InParent,
	                                      const UObject* InOwner)
		: Texture(InTexture),
		  ParentPool(InParent),
		  CurrentOwner(InOwner)
	
	{
	}
	
	~FStevesTextureRenderTargetReservation();
};


/**
 * A pool of render target textures. To save pre-creating render textures as assets, and to control the re-use of
 * these textures at runtime.
 * A pool needs to be owned by a UObject, which will in turn own the textures and so will ultimately control the
 * ultimate lifecycle of textures if not released specifically.
 * See FCompElementRenderTargetPool for inspiration
 */
struct STEVESUEHELPERS_API FStevesTextureRenderTargetPool : public FGCObject, public TSharedFromThis<FStevesTextureRenderTargetPool>
{

protected:
	/// The name of the pool. It's possible to have more than one texture pool.
	FName Name;

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

	TWeakObjectPtr<UObject> PoolOwner;
	TMultiMap<FTextureKey, TObjectPtr<UTextureRenderTarget2D>> UnreservedTextures;
	TSet<TObjectPtr<UTextureRenderTarget2D>> ReservedTextures;

	/// Weak reverse tracking of reservations, mostly for debugging
	struct FReservationInfo
	{
		FTextureKey Key;
		TWeakObjectPtr<const UObject> Owner;
		TWeakObjectPtr<UTextureRenderTarget2D> Texture;

		FReservationInfo(const FTextureKey& InKey, const UObject* InOwner, UTextureRenderTarget2D* InTexture)
			: Key(InKey),
			  Owner(InOwner),
			  Texture(InTexture)
		{
		}
	};
	TArray<FReservationInfo> Reservations;
	

	friend struct FStevesTextureRenderTargetReservation;
	/// Release a reservation on a texture, allowing it back into the pool
	/// Protected because only FStevesTextureRenderTargetReservation will need to do this.
	void ReleaseReservation(UTextureRenderTarget2D* Tex);
public:

	explicit FStevesTextureRenderTargetPool(const FName& InName, UObject* InOwner)
		: Name(InName), PoolOwner(InOwner)
	{
	}

	virtual ~FStevesTextureRenderTargetPool() override;

	const FName& GetName() const { return Name; }

	// FGCObject
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

#if ENGINE_MAJOR_VERSION >= 5
	// FGCObject
	virtual FString GetReferencerName() const override;
#endif
	/**
	 * Reserve a texture for use as a render target. This will create a new texture target if needed. 
	 * @param Size The dimensions of the texture
	 * @param Format Format of the texture
	 * @param Owner The UObject which will temporarily own this texture (mostly for debugging, this object won't in fact "own" it
	 * as per garbage collection rules, the reference is weak
	 * @return A shared pointer to a structure which holds the reservation for this texture. When that structure is
	 * destroyed, it will release the texture back to the pool.
	 */
	FStevesTextureRenderTargetReservationPtr ReserveTexture(FIntPoint Size, ETextureRenderTargetFormat Format, const UObject* Owner);

	/**
	 * Forcibly revoke reservations in this pool, either for all owners or for a specific owner.
	 * Reservations which are revoked will have their weak texture pointers invalidated.
	 * @param ForOwner If null, revoke all reservations for any owner, or if provided, just for a specific owner.
	 */
	void RevokeReservations(const UObject* ForOwner = nullptr);
	
	
	/**
	 *
	 * Destroy previously created textures and free the memory.
	 * @param bForceAndRevokeReservations If false, only destroys unreserved textures. If true, destroys reserved textures
	 * as well (the weak pointer on their reservations will cease to be valid)
	 */
	void DrainPool(bool bForceAndRevokeReservations = false);
	
};

