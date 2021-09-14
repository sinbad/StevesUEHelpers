// Copyright 2020 Old Doorways Ltd

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "StevesEditorVisComponent.generated.h"


USTRUCT(BlueprintType)
struct FStevesEditorVisLine
{
	GENERATED_BODY()

	/// Start location relative to component
	UPROPERTY(EditAnywhere)
	FVector Start;
	/// End location relative to component
	UPROPERTY(EditAnywhere)
	FVector End;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere)
	FColor Colour;

	FStevesEditorVisLine(const FVector& InStart, const FVector& InEnd, 
		const FColor& InColour)
		: Start(InStart),
		End(InEnd),
		Colour(Colour)
	{
	}

	FStevesEditorVisLine():
		Start(FVector::ZeroVector),
		End(FVector(100,0,0)),
		Colour(FColor::White)
	{
	}
};

USTRUCT(BlueprintType)
struct FStevesEditorVisCircle
{
	GENERATED_BODY()

	/// Location relative to component
	UPROPERTY(EditAnywhere)
	FVector Location;
	/// Rotation relative to component; circles will be rendered in the X/Y plane
	UPROPERTY(EditAnywhere)
	FRotator Rotation;
	/// Circle radius
	UPROPERTY(EditAnywhere)
	float Radius;
	/// The number of line segments to render the circle with
	UPROPERTY(EditAnywhere)
	int NumSegments;
	/// The colour of the line render 
	UPROPERTY(EditAnywhere)
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

/**
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
UCLASS(Blueprintable, ClassGroup="Utility", hidecategories=(Collision,Physics,Object,LOD,Lighting,TextureStreaming), meta=(DisplayName="Steves Editor Visualisation", BlueprintSpawnableComponent))
class STEVESUEHELPERS_API UStevesEditorVisComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	
	/// Circles to render
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStevesEditorVisLine> Lines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStevesEditorVisCircle> Circles;

	UStevesEditorVisComponent(const FObjectInitializer& ObjectInitializer);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	/// Needs to update on transform since proxy is detached
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return true; }
};
