// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "SmoothChangingProgressBar.generated.h"

/**
 * A specialised progress bar that can be told to change its percent smoothly instead of all at once.
 * Note: Because SetPercent isn't virtual on UProgressBar, you need to use the alternate SetPercentSmoothly
 * function instead, and call StopSmoothPercentChange to interrupt it if you need to manually set it using
 * SetPercent.
 */
UCLASS()
class STEVESUEHELPERS_API USmoothChangingProgressBar : public UProgressBar
{
	GENERATED_BODY()

protected:
	TWeakPtr<FActiveTimerHandle> SmoothChangeHandle;

	float TargetPercent;

	void UnregisterTimer();
	EActiveTimerReturnType TickPercent(double CurrTime, float DeltaTime);
public:
	/// The speed at which the progress bar changes. This value means the max percentage changes
	/// in one second. Set this to 0 to make changes instant. Changes to this value only affect
	/// the next call to SetPercentSmoothly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progress")
	float PercentChangeSpeed = 1.0f;

	/// The frequency at which we should update the bar. Set this to 0 to update every frame,
	/// or > 0 to update every X seconds (useful to save tick time for slow updates).
	/// Changes to this value only affect the next call to SetPercentSmoothly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Progress")
	float PercentChangeFrequency = 0.0f;

	/// Changes the bar percentage smoothly from its current value.
	/// Automatically interrupts any existing smooth change.
	UFUNCTION(BlueprintCallable, Category="Progress")
	void SetPercentSmoothly(float InPercent);

	/// Stop any pending smooth changes to percent
	/// Call this if you need to interrupt any current smooth change and
	UFUNCTION(BlueprintCallable, Category="Progress")
	void StopSmoothPercentChange();

	virtual void BeginDestroy() override;


	
};
