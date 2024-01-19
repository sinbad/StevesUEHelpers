#include "StevesVisualLogger.h"



void FStevesVisualLogger::InternalPolyLogfImpl(const UObject* Object,
	const FLogCategoryBase& Category,
	ELogVerbosity::Type Verbosity,
	const TArray<FVector>& Points,
	const FColor& Color,
	const uint16 Thickness)
{
	const FName CategoryName = Category.GetCategoryName();

	SCOPE_CYCLE_COUNTER(STAT_VisualLog); \
	UWorld *World = nullptr; \
	FVisualLogEntry *CurrentEntry = nullptr; \
	if (FVisualLogger::CheckVisualLogInputInternal(Object, CategoryName, Verbosity, &World, &CurrentEntry) == false)
	{
		return;
	}

	// EVisualLoggerShapeElement::Path doesn't support a label, so description is always blank
	CurrentEntry->AddElement(Points, CategoryName, Verbosity, Color, "", Thickness);
	
}
