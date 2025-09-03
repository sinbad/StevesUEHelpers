// Copyright 2020 Old Doorways Ltd

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "StevesEditorVisComponent.generated.h"


USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisLine
{
	GENERATED_BODY()

	/// Start location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisLine")
	FVector Start;
	/// End location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisLine")
	FVector End;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisLine")
	FColor Colour;

	FStevesEditorVisLine(const FVector& InStart, const FVector& InEnd,
	                     const FColor& InColour)
		: Start(InStart),
		  End(InEnd),
		  Colour(InColour)
	{
	}

	FStevesEditorVisLine():
		Start(FVector::ZeroVector),
		End(FVector(100, 0, 0)),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisCircle
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCircle")
	FVector Location;
	/// Rotation relative to component; circles will be rendered in the X/Y plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCircle")
	FRotator Rotation;
	/// Circle radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCircle")
	float Radius;
	/// The number of line segments to render the circle with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCircle")
	int NumSegments;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCircle")
	FColor Colour;

	FStevesEditorVisCircle(const FVector& InLocation, const FRotator& InRotation, float InRadius, int InNumSegments,
	                       const FColor& InColour)
		: Location(InLocation),
		  Rotation(InRotation),
		  Radius(InRadius),
		  NumSegments(InNumSegments),
		  Colour(InColour)
	{
	}

	FStevesEditorVisCircle():
		Location(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		Radius(50), NumSegments(12),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisArc
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	FVector Location;
	/// Rotation relative to component; arcs will be rendered in the X/Y plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	FRotator Rotation;
	/// Minimum angle to render arc from 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	float MinAngle;
	/// Maximum angle to render arc to 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	float MaxAngle;
	/// Circle radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	float Radius;
	/// The number of line segments to render the circle with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	int NumSegments;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisArc")
	FColor Colour;

	FStevesEditorVisArc(const FVector& InLocation, const FRotator& InRotation, float InMinAngle, float InMaxAngle,
	                    float InRadius, int InNumSegments,
	                    const FColor& InColour)
		: Location(InLocation),
		  Rotation(InRotation),
		  MinAngle(InMinAngle),
		  MaxAngle(InMaxAngle),
		  Radius(InRadius),
		  NumSegments(InNumSegments),
		  Colour(InColour)
	{
	}

	FStevesEditorVisArc():
		Location(FVector::ZeroVector),
		Rotation(FRotator::ZeroRotator),
		MinAngle(0),
		MaxAngle(180),
		Radius(50), NumSegments(12),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisSphere
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisSphere")
	FVector Location;
	/// Sphere radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisSphere")
	float Radius;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisSphere")
	FColor Colour;

	FStevesEditorVisSphere(const FVector& InLocation, float InRadius, const FColor& InColour) :
		Location(InLocation),
		Radius(InRadius),
		Colour(InColour)
	{
	}

	FStevesEditorVisSphere():
		Location(FVector::ZeroVector),
		Radius(50),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisBox
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisBox")
	FVector Location;
	/// Size of box in each axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisBox")
	FVector Size;
	/// Rotation relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisBox")
	FRotator Rotation;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisBox")
	FColor Colour;

	FStevesEditorVisBox(const FVector& InLocation, const FVector& InSize, const FRotator& InRot,
	                    const FColor& InColour) :
		Location(InLocation),
		Size(InSize),
		Rotation(InRot),
		Colour(InColour)
	{
	}

	FStevesEditorVisBox():
		Location(FVector::ZeroVector),
		Size(FVector(50, 50, 50)),
		Rotation(FRotator::ZeroRotator),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisCylinder
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCylinder")
	FVector Location;
	/// Height of cylinder
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCylinder")
	float Height;
	/// Radius of cylinder
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCylinder")
	float Radius;
	/// Rotation relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCylinder")
	FRotator Rotation;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisCylinder")
	FColor Colour;

	FStevesEditorVisCylinder(const FVector& InLocation, float InHeight, float InRadius, const FRotator& InRot,
						const FColor& InColour) :
		Location(InLocation),
		Height(InHeight),
		Radius(InRadius),
		Rotation(InRot),
		Colour(InColour)
	{
	}

	FStevesEditorVisCylinder():
		Location(FVector::ZeroVector),
		Height(50),
		Radius(10),
		Rotation(FRotator::ZeroRotator),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisCapsule : public FStevesEditorVisCylinder
{
	GENERATED_BODY()

	FStevesEditorVisCapsule(const FVector& InLocation, float InHeight, float InRadius, const FRotator& InRot,
						const FColor& InColour) : FStevesEditorVisCylinder(InLocation, InHeight, InRadius, InRot, InColour)
	{
	}

	FStevesEditorVisCapsule() : FStevesEditorVisCylinder()
	{
	}

};

USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesEditorVisMesh
{
	GENERATED_BODY()

	/// The mesh do display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	TObjectPtr<UStaticMesh> Mesh;
	/// Location relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	FVector Location;
	/// Scale of the mesh compared to component scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	FVector Scale;
	/// Rotation relative to component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	FRotator Rotation;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	FColor Colour;
	/// Whether to use the lowest detail LOD for vis 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisMesh")
	bool bUseLowestLOD = true;

	FStevesEditorVisMesh(UStaticMesh* InMesh,
	                     const FVector& InLocation,
	                     const FVector& InScale,
	                     const FRotator& InRot,
	                     const FColor& InColour,
	                     bool InUseLowestLOD = true) :
		Mesh(InMesh),
		Location(InLocation),
		Scale(InScale),
		Rotation(InRot),
		Colour(InColour),
		bUseLowestLOD(InUseLowestLOD)
	{
	}

	FStevesEditorVisMesh() : Mesh(nullptr),
	                         Location(FVector::ZeroVector),
	                         Scale(FVector::OneVector),
	                         Rotation(FRotator::ZeroRotator),
	                         Colour(FColor::White)
	{
	}
};

/**
 *
 * A component that solely exists to provide arbitrary editor visualisation when not selected.
 * FComponentVisualizer can only display visualisation when selected. 
 * To display vis on an unselected object, you need a UPrimitiveComponent, and sometimes you don't want/need one of those
 * in your actor. Instead, add UStevesEditorVisComponent at construction of your actor, or registration of another component,
 * but only in a WITH_EDITOR block. Then, get nice visualisation of your actor/component without making more invasive changes
 * to your code.
 *
 * If you want to, you can add this to your Blueprints too. This component automatically marks itself as "visualisation
 * only" so shouldn't have a runtime impact.
 */
UCLASS(Blueprintable, ClassGroup="Utility", hidecategories=(Collision,Physics,Object,LOD,Lighting,TextureStreaming),
	meta=(DisplayName="Steves Editor Visualisation", BlueprintSpawnableComponent))
class STEVESUEHELPERS_API UStevesEditorVisComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	/// Circles to render
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisLine> Lines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisLine> Arrows;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisCircle> Circles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisArc> Arcs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisSphere> Spheres;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisBox> Boxes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisCylinder> Cylinders;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisCapsule> Capsules;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StevesVisComponent")
	TArray<FStevesEditorVisMesh> Meshes;

	UStevesEditorVisComponent(const FObjectInitializer& ObjectInitializer);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	/// Needs to update on transform since proxy is detached
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return true; }
};
