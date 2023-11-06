// Copyright Steve Streeting
// Licensed under the MIT License (see License.txt)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StevesLightFlicker.generated.h"


UENUM(BlueprintType)
enum class EStevesLightFlickerPattern : uint8
{
	Flicker1,
	Flicker2,
	SlowStrongPulse,
	Candle1,
	Candle2,
	Candle3,
	FastStrobe,
	SlowStrobe,
	GentlePulse1,
	FlourescentFlicker,
	SlowPulseNoBlack,

	Custom
	
};
/**
 * Helper class to get lighting flicker curves
 */
UCLASS()
class STEVESUEHELPERS_API UStevesLightFlickerHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:
	static TMap<EStevesLightFlickerPattern, FRichCurve> Curves;
	static TMap<FString, FRichCurve> CustomCurves;
	static FCriticalSection CriticalSection;
	static const TMap<EStevesLightFlickerPattern, FString> QuakeCurveSources;
	

	static void BuildCurve(EStevesLightFlickerPattern CurveType, FRichCurve& OutCurve);
	static void BuildCurve(const FString& QuakeCurveChars, FRichCurve& OutCurve);

public:
	/**
	 * Directly evaluate a lighting curve. Alternatively, see ULightingCurveComponent.
	 * @param CurveType The type of curve
	 * @param Time The time index (0..1 period)
	 * @return Normalised value of the curve at this time
	 */
	UFUNCTION(BlueprintPure, Category="Lighting Curves")
	static float EvaluateLightCurve(EStevesLightFlickerPattern CurveType, float Time);

	static const FRichCurve& GetLightCurve(EStevesLightFlickerPattern CurveType);
	static const FRichCurve& GetLightCurve(const FString& CurveStr);

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLightFlickerUpdate, float, LightValue);
/** 
 * This is like a generated version of TimelineComponent, providing a generated lighting curve.
 */
UCLASS(Blueprintable, ClassGroup=(Lights), meta=(BlueprintSpawnableComponent))
class UStevesLightFlickerComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="Light Flicker")
	EStevesLightFlickerPattern FlickerPattern = EStevesLightFlickerPattern::Candle1;

	/// If using a custom pattern, provide your own Quake-style string of letters, a-z (a = 0, m = 1, z = 2)
	UPROPERTY(EditAnywhere, Category="Light Flicker")
	FString CustomFlickerPattern;

	/// Max output intensity multiplier value. Defaults to 2 since that's what Quake used!
	/// We can *very slightly* exceed this max with 'z' as per standard Quake where z was 2.08
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Flicker")
	float MaxValue = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Flicker")
	float MinValue = 0;

	/// Playback speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light Flicker")
	float Speed = 1;

	/// Whether to auto-start
	UPROPERTY(EditAnywhere, Category="Light Flicker")
	bool bAutoPlay = true;

	UPROPERTY(ReplicatedUsing=OnRep_TimePos)
	float TimePos;
	float CurrentValue;
	
	const FRichCurve* Curve;

	UFUNCTION()
	void OnRep_TimePos();
	void ValueUpdate();

public:

	UPROPERTY(BlueprintAssignable)
	FOnLightFlickerUpdate OnLightFlickerUpdate;

	UFUNCTION(BlueprintCallable)
	void Play(bool bResetTime = false);
	UFUNCTION(BlueprintCallable)
	void Pause();
	UFUNCTION(BlueprintPure)
	float GetCurrentValue() const;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};