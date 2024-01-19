#pragma once
#include "CoreMinimal.h"
#include "VisualLogger/VisualLogger.h"

class STEVESUEHELPERS_API FStevesVisualLogger
{
protected:
	static void InternalPolyLogfImpl(const UObject* LogOwner,
	                                 const FLogCategoryBase& Category,
	                                 ELogVerbosity::Type Verbosity,
	                                 const TArray<FVector>& Points,
	                                 const FColor& Color,
	                                 const uint16 Thickness);

	
	static void InternalPolyLogf(const UObject* LogOwner,
	                         const FLogCategoryBase& Category,
	                         ELogVerbosity::Type Verbosity,
	                         const FVector& Center,
	                         const FVector& UpAxis,
	                         const float OuterRadius,
	                         const float InnerRadius,
	                         const int NumPoints,
	                         const FColor& Color,
	                         const uint16 Thickness)
	{

		const bool bIsStar = !FMath::IsNearlyEqual(OuterRadius, InnerRadius);

		const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, UpAxis);
		const FVector YAxis = Rotation.RotateVector(FVector::YAxisVector);

		const int Count = bIsStar ? NumPoints * 2 : NumPoints;
		const float RotInc = UE_TWO_PI / Count;
		TArray<FVector> Vertices;
		// Start
		Vertices.Add(Center + YAxis * OuterRadius);
		for (int i = 1; i < Count; ++i)
		{
			const float Scale = bIsStar && (i % 2) != 0 ? InnerRadius : OuterRadius;
			const FQuat Rot = FQuat(UpAxis, RotInc * i);
			const FVector V = Center + Rot.RotateVector(YAxis * Scale);
			Vertices.Add(V);
		}
		// End
		Vertices.Add(Center + YAxis * OuterRadius);

		InternalPolyLogfImpl(LogOwner, Category, Verbosity, Vertices, Color, Thickness);
	}
	
public:
	static void TriangleLogf(const UObject* LogOwner, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FVector& Center, const FVector& UpAxis, const float Radius, const FColor& Color, const uint16 Thickness)
	{
		InternalPolyLogf(LogOwner, Category, Verbosity, Center, UpAxis, Radius, Radius, 3, Color, Thickness);
	}

	static void SquareLogf(const UObject* LogOwner, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FVector& Center, const FVector& UpAxis, const float Radius, const FColor& Color, const uint16 Thickness)
	{
		InternalPolyLogf(LogOwner, Category, Verbosity, Center, UpAxis, Radius, Radius, 4, Color, Thickness);
	}

	static void PolyLogf(const UObject* LogOwner, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FVector& Center, const FVector& UpAxis, const float Radius, const int NumPoints, const FColor& Color, const uint16 Thickness)
	{
		InternalPolyLogf(LogOwner, Category, Verbosity, Center, UpAxis, Radius, Radius, NumPoints, Color, Thickness);
	}

	static void StarLogf(const UObject* LogOwner, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FVector& Center, const FVector& UpAxis, const float OuterRadius, const float InnerRadius, const int NumPoints, const FColor& Color, const uint16 Thickness)
	{
		InternalPolyLogf(LogOwner, Category, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, Thickness);
	}
};

#if ENABLE_VISUAL_LOG

// 2D Triangle shape
#define UE_VLOG_TRIANGLE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) if(FVisualLogger::IsRecording()) FStevesVisualLogger::TriangleLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, Color, 0)
#define UE_CVLOG_TRIANGLE(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_TRIANGLE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, 0);} 
#define UE_VLOG_TRIANGLE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness) if(FVisualLogger::IsRecording()) FStevesVisualLogger::TriangleLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, Color, Thickness)
#define UE_CVLOG_TRIANGLE_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_TRIANGLE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness);} 

// 2D Square shape
#define UE_VLOG_SQUARE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) if(FVisualLogger::IsRecording()) FStevesVisualLogger::SquareLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, Color, 0)
#define UE_CVLOG_SQUARE(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_SQUARE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, 0);} 
#define UE_VLOG_SQUARE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness) if(FVisualLogger::IsRecording()) FStevesVisualLogger::SquareLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, Color, Thickness)
#define UE_CVLOG_SQUARE_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_SQUARE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color, Thickness);} 

// 2D poly shape
#define UE_VLOG_POLY(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color) if(FVisualLogger::IsRecording()) FStevesVisualLogger::PolyLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, NumPoints, Color, 0)
#define UE_CVLOG_POLY(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_POLY(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color, 0);} 
#define UE_VLOG_POLY_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color, Thickness) if(FVisualLogger::IsRecording()) FStevesVisualLogger::PolyLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, Radius, NumPoints, Color, Thickness)
#define UE_CVLOG_POLY_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color, Thickness)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_POLY_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color, Thickness);} 

// 2D star shape
#define UE_VLOG_STAR(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color) if(FVisualLogger::IsRecording()) FStevesVisualLogger::StarLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, 0)
#define UE_CVLOG_STAR(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_STAR(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, 0);} 
#define UE_VLOG_STAR_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, Thickness) if(FVisualLogger::IsRecording()) FStevesVisualLogger::StarLogf(LogOwner, CategoryName, ELogVerbosity::Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, Thickness)
#define UE_CVLOG_STAR_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, Thickness)  if(FVisualLogger::IsRecording() && Condition) {UE_VLOG_STAR_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color, Thickness);} 

#else
#define UE_VLOG_TRIANGLE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) 
#define UE_CVLOG_TRIANGLE(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  
#define UE_VLOG_TRIANGLE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) 
#define UE_CVLOG_TRIANGLE_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  

// Square shape
#define UE_VLOG_SQUARE(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) 
#define UE_CVLOG_SQUARE(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  
#define UE_VLOG_SQUARE_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color) 
#define UE_CVLOG_SQUARE_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, Color)  

// 2D poly shape
#define UE_VLOG_POLY(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color) 
#define UE_CVLOG_POLY(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color)  
#define UE_VLOG_POLY_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color) 
#define UE_CVLOG_POLY_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, Radius, NumPoints, Color)  

// Any 2D star shape
#define UE_VLOG_STAR(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color) 
#define UE_CVLOG_STAR(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color)  
#define UE_VLOG_STAR_THICK(LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color) 
#define UE_CVLOG_STAR_THICK(Condition, LogOwner, CategoryName, Verbosity, Center, UpAxis, OuterRadius, InnerRadius, NumPoints, Color)  

#endif