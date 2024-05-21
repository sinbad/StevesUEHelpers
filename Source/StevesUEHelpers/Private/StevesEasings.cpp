#include "StevesEasings.h"

float UStevesEasings::EaseAlpha(float InAlpha, EStevesEaseFunction Func)
{
	constexpr float BackC1 = 1.70158f;
	constexpr float BackC2 = BackC1 * 1.525f;
	constexpr float BackC3 = BackC1 + 1.f;
	constexpr float ElasticC4 = UE_TWO_PI / 3.f;
	constexpr float ElasticC5 = UE_TWO_PI / 4.5;

	switch(Func)
	{
	default:
	case EStevesEaseFunction::Linear:
		return InAlpha;
	case EStevesEaseFunction::EaseIn_Sine:
		return 1.f - FMath::Cos(InAlpha * UE_HALF_PI);
	case EStevesEaseFunction::EaseOut_Sine:
		return FMath::Sin(InAlpha * UE_HALF_PI);
	case EStevesEaseFunction::EaseInOut_Sine:
		return -(FMath::Cos(UE_PI * InAlpha) - 1.f) / 2.f;
	case EStevesEaseFunction::EaseIn_Quad:
		return InAlpha*InAlpha;
	case EStevesEaseFunction::EaseOut_Quad:
		return 1.f - (1.f - InAlpha) * (1.f - InAlpha);
	case EStevesEaseFunction::EaseInOut_Quad:
		return InAlpha < 0.5f ? 2.f * InAlpha * InAlpha : 1.f - FMath::Pow(-2.f * InAlpha + 2.f, 2.f) / 2.f;
	case EStevesEaseFunction::EaseIn_Cubic:
		return FMath::Pow(InAlpha, 3);
	case EStevesEaseFunction::EaseOut_Cubic:
		return 1.f - FMath::Pow(1.f - InAlpha, 3);
	case EStevesEaseFunction::EaseInOut_Cubic:
		return InAlpha < 0.5f ? 4.f * FMath::Pow(InAlpha, 3) : 1.f - FMath::Pow(-2.f * InAlpha + 2.f, 3) / 2.f;
	case EStevesEaseFunction::EaseIn_Quart:
		return FMath::Pow(InAlpha, 4);
	case EStevesEaseFunction::EaseOut_Quart:
		return 1 - FMath::Pow(1.f - InAlpha, 4.f);
	case EStevesEaseFunction::EaseInOut_Quart:
		return InAlpha < 0.5f ? 8.f * FMath::Pow(InAlpha, 4) : 1.f - FMath::Pow(-2.f * InAlpha + 2.f, 4) / 2.f;
	case EStevesEaseFunction::EaseIn_Quint:
		return FMath::Pow(InAlpha, 5);
	case EStevesEaseFunction::EaseOut_Quint:
		return 1 - FMath::Pow(1.f - InAlpha, 5);
	case EStevesEaseFunction::EaseInOut_Quint:
		return InAlpha < 0.5f ? 16.f * FMath::Pow(InAlpha, 5) : 1.f - FMath::Pow(-2.f * InAlpha + 2.f, 5) / 2.f;
	case EStevesEaseFunction::EaseIn_Expo:
		return InAlpha <= 0 ? 0 : FMath::Pow(2.f, 10.f * InAlpha - 10.f);
	case EStevesEaseFunction::EaseOut_Expo:
		return InAlpha >= 1.f ? 1.f : 1.f - FMath::Pow(2.f, -10.f * InAlpha);
	case EStevesEaseFunction::EaseInOut_Expo:
		if (InAlpha <= 0.f)
			return 0;
		if (InAlpha >= 1.f)
			return 1;
		return InAlpha < 0.5f
			       ? FMath::Pow(2.f, 20.f * InAlpha - 10.f) / 2.f
			       : (2.f - FMath::Pow(2.f, -20.f * InAlpha + 10.f)) / 2.f;
	case EStevesEaseFunction::EaseIn_Circ:
		return 1.f - FMath::Sqrt(1.f - FMath::Pow(InAlpha, 2));
	case EStevesEaseFunction::EaseOut_Circ:
		return FMath::Sqrt(1.f - FMath::Pow(InAlpha - 1.f, 2));
	case EStevesEaseFunction::EaseInOut_Circ:
		return InAlpha < 0.5f
			       ? (1.f - FMath::Sqrt(1.f - FMath::Pow(2.f * InAlpha, 2))) / 2.f
			       : (FMath::Sqrt(1.f - FMath::Pow(-2.f * InAlpha + 2.f, 2)) + 1.f) / 2.f;
	case EStevesEaseFunction::EaseIn_Back:
		return BackC3 * FMath::Pow(InAlpha, 3) - BackC1 * InAlpha * InAlpha;
	case EStevesEaseFunction::EaseOut_Back:
		return 1.f + BackC3 * FMath::Pow(InAlpha - 1.f, 3) + BackC1 * FMath::Pow(InAlpha - 1.f, 2.f);
	case EStevesEaseFunction::EaseInOut_Back:
		return InAlpha < 0.5f
			       ? (FMath::Pow(2.f * InAlpha, 2) * ((BackC2 + 1.f) * 2.f * InAlpha - BackC2)) / 2.f
			       : (FMath::Pow(2.f * InAlpha - 2.f, 2) * ((BackC2 + 1) * (InAlpha * 2.f - 2.f) + BackC2) + 2.f) / 2.f;
	case EStevesEaseFunction::EaseIn_Elastic:
		if (InAlpha <= 0.f)
			return 0;

		if (InAlpha >= 1.f)
			return 1;
		return -FMath::Pow(2.f, 10.f * InAlpha - 10.f) * FMath::Sin((InAlpha * 10.f - 10.75f) * ElasticC4);
	case EStevesEaseFunction::EaseOut_Elastic:
		if (InAlpha <= 0.f)
			return 0;
		if (InAlpha >= 1.f)
			return 1;
		return FMath::Pow(2.f, -10.f * InAlpha) * FMath::Sin((InAlpha * 10.f - 0.75f) * ElasticC4) + 1.f;
	case EStevesEaseFunction::EaseInOut_Elastic:
		if (InAlpha <= 0.f)
			return 0;
		if (InAlpha >= 1.f)
			return 1;
		return InAlpha < 0.5f
			       ? -(FMath::Pow(2.f, 20.f * InAlpha - 10.f) * FMath::Sin((20.f * InAlpha - 11.125f) * ElasticC5)) /
			       2.f
			       : (FMath::Pow(2.f, -20.f * InAlpha + 10.f) * FMath::Sin((20.f * InAlpha - 11.125f) * ElasticC5)) /
			       2.f + 1.f;
	case EStevesEaseFunction::EaseIn_Bounce:
		return 1 - EaseAlpha(1 - InAlpha, EStevesEaseFunction::EaseOut_Bounce);
	case EStevesEaseFunction::EaseOut_Bounce:
		{
			constexpr float n1 = 7.5625f;
			constexpr float d1 = 2.75f;

			if (InAlpha < 1.f / d1) { return n1 * InAlpha * InAlpha; }
			else if (InAlpha < 2.f / d1)
			{
				const float newAlpha = InAlpha - 1.5f/d1;
				return n1 * newAlpha * newAlpha + 0.75f;
			}
			else if (InAlpha < 2.5f / d1)
			{
				const float newAlpha = InAlpha - 2.25f/d1;
				return n1 * newAlpha * newAlpha + 0.9375f;
			}
			else
			{
				const float newAlpha = InAlpha - 2.625f/d1;
				return n1 * newAlpha * newAlpha + 0.984375f;
			}
		}
	case EStevesEaseFunction::EaseInOut_Bounce:
		return InAlpha < 0.5f
			       ? (1.f - EaseAlpha(1.f - 2.f * InAlpha, EStevesEaseFunction::EaseOut_Bounce)) / 2.f
			       : (1.f + EaseAlpha(2.f * InAlpha - 1.f, EStevesEaseFunction::EaseOut_Bounce)) / 2.f;
	}
}
