// Copyright Steve Streeting
// Licensed under the MIT License (see License.txt)
#include "StevesLightFlicker.h"

#include "Net/UnrealNetwork.h"

TMap<EStevesLightFlickerPattern, FRichCurve> UStevesLightFlickerHelper::Curves;
TMap<FString, FRichCurve> UStevesLightFlickerHelper::CustomCurves;
FCriticalSection UStevesLightFlickerHelper::CriticalSection;

// Quake lighting flicker functions
// https://github.com/id-Software/Quake/blob/bf4ac424ce754894ac8f1dae6a3981954bc9852d/qw-qc/world.qc#L328-L372
const TMap<EStevesLightFlickerPattern, FString> UStevesLightFlickerHelper::QuakeCurveSources {
	{ EStevesLightFlickerPattern::Flicker1, TEXT("mmnmmommommnonmmonqnmmo") },
	{ EStevesLightFlickerPattern::SlowStrongPulse, TEXT("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba") },
	{ EStevesLightFlickerPattern::Candle1, TEXT("mmmmmaaaaammmmmaaaaaabcdefgabcdefg") },
	{ EStevesLightFlickerPattern::FastStrobe, TEXT("mamamamamama") },
	{ EStevesLightFlickerPattern::GentlePulse1, TEXT("jklmnopqrstuvwxyzyxwvutsrqponmlkj") },
	{ EStevesLightFlickerPattern::Flicker2, TEXT("nmonqnmomnmomomno") },
	{ EStevesLightFlickerPattern::Candle2, TEXT("mmmaaaabcdefgmmmmaaaammmaamm") },
	{ EStevesLightFlickerPattern::Candle3, TEXT("mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa") },
	{ EStevesLightFlickerPattern::SlowStrobe, TEXT("aaaaaaaazzzzzzzz") },
	{ EStevesLightFlickerPattern::FlourescentFlicker, TEXT("mmamammmmammamamaaamammma") },
	{ EStevesLightFlickerPattern::SlowPulseNoBlack, TEXT("abcdefghijklmnopqrrqponmlkjihgfedcba") },
};

float UStevesLightFlickerHelper::EvaluateLightCurve(EStevesLightFlickerPattern CurveType, float Time)
{
	return GetLightCurve(CurveType).Eval(Time);
}

const FRichCurve& UStevesLightFlickerHelper::GetLightCurve(EStevesLightFlickerPattern CurveType)
{
	FScopeLock ScopeLock(&CriticalSection);

	if (auto pCurve = Curves.Find(CurveType))
	{
		return *pCurve;
	}

	auto& Curve = Curves.Emplace(CurveType);
	BuildCurve(CurveType, Curve);
	return Curve;
}

const FRichCurve& UStevesLightFlickerHelper::GetLightCurve(const FString& CurveStr)
{
	FScopeLock ScopeLock(&CriticalSection);

	if (auto pCurve = CustomCurves.Find(CurveStr))
	{
		return *pCurve;
	}

	auto& Curve = CustomCurves.Emplace(CurveStr);
	BuildCurve(CurveStr, Curve);
	return Curve;
}

void UStevesLightFlickerHelper::BuildCurve(EStevesLightFlickerPattern CurveType, FRichCurve& OutCurve)
{
	if (auto pTxt = QuakeCurveSources.Find(CurveType))
	{
		BuildCurve(*pTxt, OutCurve);
	}
	
}

void UStevesLightFlickerHelper::BuildCurve(const FString& QuakeCurveChars, FRichCurve& OutCurve)
{
	OutCurve.Reset();

	for (int i = 0; i < QuakeCurveChars.Len(); ++i)
	{
		// We actually build the curve a..z = 0..1, and then use a default max value of 2 to restore the original behaviour.
		// Actually the curve is 0..1.04 due to original behaviour that z is 2.08 not 2
		const int CharIndex = QuakeCurveChars[i] - 'a';
		const float Val = (float)CharIndex / 24.f; // to ensure m==1, z==2.08 (rescaled to half that so 0..1.04)
		// Quake default was each character was 0.1s
		OutCurve.AddKey(i * 0.1f, Val);
	}

	// To catch empty
	if (QuakeCurveChars.IsEmpty())
	{
		OutCurve.AddKey(0, 1);
	}
}

UStevesLightFlickerComponent::UStevesLightFlickerComponent(const FObjectInitializer& Initializer):
	Super(Initializer),
	TimePos(0),
	CurrentValue(0),
	Curve(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UStevesLightFlickerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FlickerPattern == EStevesLightFlickerPattern::Custom)
	{
		Curve = &UStevesLightFlickerHelper::GetLightCurve(CustomFlickerPattern);
	}
	else
	{
		Curve = &UStevesLightFlickerHelper::GetLightCurve(FlickerPattern);
	}
	TimePos = 0;
	if (bAutoPlay)
	{
		Play();
	}
}

void UStevesLightFlickerComponent::ValueUpdate()
{
	CurrentValue = FMath::Lerp(MinValue, MaxValue, Curve->Eval(TimePos));
	OnLightFlickerUpdate.Broadcast(CurrentValue);
}

void UStevesLightFlickerComponent::Play(bool bResetTime)
{
	if (GetOwnerRole() == ROLE_Authority || !GetIsReplicated())
	{
		if (bResetTime)
		{
			TimePos = 0;
		}
		ValueUpdate();

		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UStevesLightFlickerComponent::Pause()
{
	if (GetOwnerRole() == ROLE_Authority || !GetIsReplicated())
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

float UStevesLightFlickerComponent::GetCurrentValue() const
{
	return CurrentValue;
}

void UStevesLightFlickerComponent::OnRep_TimePos()
{
	ValueUpdate();
}

void UStevesLightFlickerComponent::TickComponent(float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimePos += DeltaTime * Speed;
	const float MaxTime = Curve->GetLastKey().Time;
	while (TimePos > MaxTime)
	{
		TimePos -= MaxTime;
	}
	ValueUpdate();
}

void UStevesLightFlickerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStevesLightFlickerComponent, TimePos);
}
