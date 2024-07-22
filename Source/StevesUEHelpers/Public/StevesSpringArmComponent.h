
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "StevesSpringArmComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEVESUEHELPERS_API UStevesSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()
public:
	/// Whether to move the camera smoothly to avoid collisions rather than jumping instantly
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraCollision)
	uint32 bEnableSmoothCollisionAvoidance : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraCollision)
	float SmoothCollisionAvoidanceSpeed = 5;
	
protected:
	TOptional<float> PrevArmLength;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category=CameraCollision)
	bool bVisualLogCameraCollision = false;
#endif

public:
	UStevesSpringArmComponent();

protected:
	virtual FVector BlendLocations(const FVector& DesiredArmLocation,
		const FVector& TraceHitLocation,
		bool bHitSomething,
		float DeltaTime) override;
};

