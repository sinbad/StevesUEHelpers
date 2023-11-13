#pragma once

#include "CoreMinimal.h"
#include "StevesEasings.generated.h"

/// Easing functions
/// See https://easings.net/
/// Could have used UE EEasingFunc but it's missing some nice options like back/elastic 
UENUM(BlueprintType)
enum class EStevesEaseFunction : uint8
{
	Linear,
	EaseIn_Sine,
	EaseOut_Sine,
	EaseInOut_Sine,
	EaseIn_Quad,
	EaseOut_Quad,
	EaseInOut_Quad,
	EaseIn_Cubic,
	EaseOut_Cubic,
	EaseInOut_Cubic,
	EaseIn_Quart,
	EaseOut_Quart,
	EaseInOut_Quart,
	EaseIn_Quint,
	EaseOut_Quint,
	EaseInOut_Quint,
	EaseIn_Expo,
	EaseOut_Expo,
	EaseInOut_Expo,
	EaseIn_Circ,
	EaseOut_Circ,
	EaseInOut_Circ,
	EaseIn_Back,
	EaseOut_Back,
	EaseInOut_Back,
	EaseIn_Elastic,
	EaseOut_Elastic,
	EaseInOut_Elastic,
	EaseIn_Bounce,
	EaseOut_Bounce,
	EaseInOut_Bounce
};

UCLASS()
class STEVESUEHELPERS_API UStevesEasings : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Convert a linear alpha value into an eased alpha value using an easing function 
	 * @param InAlpha The input linear alpha
	 * @param Func The easing function
	 * @return The eased version of the alpha
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static float EaseAlpha(float InAlpha, EStevesEaseFunction Func);

	/**
	 * Interpolate with easing function support 
	 * @param A Input Value from
	 * @param B Input Value to
	 * @param Alpha Value between 0 and 1
	 * @param Func Easing function
	 * @return Interpolated value
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static float EaseFloat(float A, float B, float Alpha, EStevesEaseFunction Func)
	{
		return A + EaseAlpha(Alpha, Func) * (B - A);
	}
	/**
	 * Interpolate with easing function support 
	 * @param A Input Value from
	 * @param B Input Value to
	 * @param Alpha Value between 0 and 1
	 * @param Func Easing function
	 * @return Interpolated value
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static FVector EaseVector(const FVector& A, const FVector& B, float Alpha, EStevesEaseFunction Func)
	{
		return A + EaseAlpha(Alpha, Func) * (B - A);
	}
	/**
	 * Interpolate with easing function support 
	 * @param A Input Value from
	 * @param B Input Value to
	 * @param Alpha Value between 0 and 1
	 * @param Func Easing function
	 * @return Interpolated value
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static FRotator EaseRotator(const FRotator& A, const FRotator& B, float Alpha, EStevesEaseFunction Func, bool bShortest)
	{
		if (bShortest)
			return EaseQuat(FQuat(A), FQuat(B), Alpha, Func).Rotator();

		return A + EaseAlpha(Alpha, Func) * (B - A);
	}
	/**
	 * Interpolate with easing function support 
	 * @param A Input Value from
	 * @param B Input Value to
	 * @param Alpha Value between 0 and 1
	 * @param Func Easing function
	 * @return Interpolated value
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static FQuat EaseQuat(const FQuat& A, const FQuat& B, float Alpha, EStevesEaseFunction Func)
	{
		return FQuat::Slerp(A, B, EaseAlpha(Alpha, Func));
	}	
	/**
	 * Interpolate with easing function support 
	 * @param A Input Value from
	 * @param B Input Value to
	 * @param Alpha Value between 0 and 1
	 * @param Func Easing function
	 * @return Interpolated value
	 */
	UFUNCTION(BlueprintCallable, Category="StevesEaseMath")
	static FTransform EaseTransform(const FTransform& A, const FTransform& B, float Alpha, EStevesEaseFunction Func)
	{
		return FTransform(
			EaseQuat(A.GetRotation(), B.GetRotation(), Alpha, Func),
			EaseVector(A.GetLocation(), B.GetLocation(), Alpha, Func),
			EaseVector(A.GetScale3D(), B.GetScale3D(), Alpha, Func));
	}	
	
	
};
