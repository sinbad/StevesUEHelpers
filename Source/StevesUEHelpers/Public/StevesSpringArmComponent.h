
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

	TOptional<FVector> SmoothTargetOffsetTarget;
	float SmoothTargetOffsetSpeed;
	TOptional<FVector> SmoothSocketOffsetTarget;
	float SmoothSocketOffsetSpeed;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category=CameraCollision)
	bool bVisualLogCameraCollision = false;
#endif

public:
	UStevesSpringArmComponent();

	/// Smoothly change the target offset, instead of jumping
	UFUNCTION(BlueprintCallable, Category="SpringArm")
	void SetTargetOffsetSmooth(const FVector& NewTargetOffset, float Speed = 5);
	/// Interrupt a smooth target offset change, freeze where we are
	UFUNCTION(BlueprintCallable, Category="SpringArm")
	void CancelTargetOffsetSmooth();

	/// Smoothly change the socket offset, instead of jumping
	UFUNCTION(BlueprintCallable, Category="SpringArm")
	void SetSocketOffsetSmooth(const FVector& NewSocketOffset, float Speed = 5);
	/// Interrupt a smooth socket offset change, freeze where we are
	UFUNCTION(BlueprintCallable, Category="SpringArm")
	void CancelSocketOffsetSmooth();

protected:
	virtual void UpdateDesiredArmLocation(bool bDoTrace,
		bool bDoLocationLag,
		bool bDoRotationLag,
		float DeltaTime) override;
	virtual FVector BlendLocations(const FVector& DesiredArmLocation,
	                               const FVector& TraceHitLocation,
	                               bool bHitSomething,
	                               float DeltaTime) override;
};

