#pragma once
#include "Engine/DataTable.h"

#include "StevesFixedDataTableRowHandle.generated.h"

/// Just a type to denote that this table row handle should be edited differently
/// 
/// Usage:
///    UPROPERTY(EditAnywhere, meta=(DataTable="/Game/Data/DT_YourData"))
///    FStevesFixedDataTableRowHandle Row;
///    
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FStevesFixedDataTableRowHandle : public FDataTableRowHandle
{
	GENERATED_USTRUCT_BODY()
};
	