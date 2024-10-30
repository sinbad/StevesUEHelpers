#include "StevesUI/SmoothChangingProgressBar.h"

void USmoothChangingProgressBar::SetPercentSmoothly(float InPercent)
{
	UnregisterTimer();

	TargetPercent = InPercent;
	const float Curr = GetPercent();
	if (!FMath::IsNearlyEqual(Curr, TargetPercent))
	{
		if (FMath::IsNearlyZero(PercentChangeSpeed) || !MyProgressBar.IsValid())
		{
			SetPercent(InPercent);
		}
		else
		{
			SmoothChangeHandle = MyProgressBar->RegisterActiveTimer(
				PercentChangeFrequency,
				FWidgetActiveTimerDelegate::CreateUObject(this, &USmoothChangingProgressBar::TickPercent));
		}
	}
}

void USmoothChangingProgressBar::StopSmoothPercentChange()
{
	UnregisterTimer();
}

void USmoothChangingProgressBar::BeginDestroy()
{
	Super::BeginDestroy();

	UnregisterTimer();
}

void USmoothChangingProgressBar::UnregisterTimer()
{
	if (SmoothChangeHandle.IsValid() && MyProgressBar.IsValid())
	{
		MyProgressBar->UnRegisterActiveTimer(SmoothChangeHandle.Pin().ToSharedRef());
		SmoothChangeHandle.Reset();
	}
}

EActiveTimerReturnType USmoothChangingProgressBar::TickPercent(double CurrTime, float DeltaTime)
{
	const float CurrPercent = GetPercent();
	const float Direction = FMath::Sign(TargetPercent - CurrPercent);
	const float Change = DeltaTime * Direction * PercentChangeSpeed;

	const float NewPercent = Direction > 0
		                         ? FMath::Min(CurrPercent + Change, TargetPercent)
		                         : FMath::Max(CurrPercent + Change, TargetPercent);
	SetPercent(NewPercent);

	// Stop this if reached target (will unregister itself)
	if (FMath::IsNearlyEqual(TargetPercent, GetPercent()))
	{
		return EActiveTimerReturnType::Stop;
	}
	return EActiveTimerReturnType::Continue;
}
