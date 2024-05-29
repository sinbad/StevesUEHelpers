#include "StevesFixedDataTableCustomisationLayout.h"

#include "AssetRegistry/AssetData.h"
#include "Containers/Map.h"
#include "DataTableEditorUtils.h"
#include "Delegates/Delegate.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Commands/UIAction.h"
#include "HAL/Platform.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Misc/Attribute.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Object.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"

class SToolTip;

#define LOCTEXT_NAMESPACE "FSsFixedDataTableCustomisationLayout"

void FStevesFixedDataTableCustomisationLayout::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	DataTablePropertyHandle = InStructPropertyHandle->GetChildHandle("DataTable");
	RowNamePropertyHandle = InStructPropertyHandle->GetChildHandle("RowName");

	if (InStructPropertyHandle->HasMetaData(TEXT("DataTable")))
	{
		// Find data table from asset ref
		const FString& DataTablePath = InStructPropertyHandle->GetMetaData(TEXT("DataTable"));
		if (UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath, nullptr))
		{
			UObject* Existing = nullptr;
			const bool TablePicked = DataTablePropertyHandle->GetValue(Existing) == FPropertyAccess::Success;
			if (!TablePicked || Existing != DataTable)
			{
				DataTablePropertyHandle->SetValue(DataTable);
			}
		}
		else
		{
			UE_LOG(LogDataTable, Warning, TEXT("No Datatable found at %s"), *DataTablePath);
		}
	}
	else
	{
		UE_LOG(LogDataTable, Warning, TEXT("No Datatable meta tag present on property %s"), *InStructPropertyHandle->GetPropertyDisplayName().ToString());
	}

	

	FPropertyComboBoxArgs ComboArgs(RowNamePropertyHandle, 
			FOnGetPropertyComboBoxStrings::CreateSP(this, &FStevesFixedDataTableCustomisationLayout::OnGetRowStrings), 
			FOnGetPropertyComboBoxValue::CreateSP(this, &FStevesFixedDataTableCustomisationLayout::OnGetRowValueString));
	ComboArgs.ShowSearchForItemCount = 1;


	TSharedRef<SWidget> BrowseTableButton = PropertyCustomizationHelpers::MakeBrowseButton(
		FSimpleDelegate::CreateSP(this, &FStevesFixedDataTableCustomisationLayout::BrowseTableButtonClicked),
		LOCTEXT("SsBrowseToDatatable", "Browse to DataTable in Content Browser"));
	HeaderRow
	.NameContent()
	[
		InStructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MaxDesiredWidth(0.0f) // don't constrain the combo button width
	[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				PropertyCustomizationHelpers::MakePropertyComboBox(ComboArgs)
			]
			+SHorizontalBox::Slot()
			.Padding(2.0f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				BrowseTableButton
			]
	];	;

	FDataTableEditorUtils::AddSearchForReferencesContextMenu(HeaderRow, FExecuteAction::CreateSP(this, &FStevesFixedDataTableCustomisationLayout::OnSearchForReferences));
}

void FStevesFixedDataTableCustomisationLayout::BrowseTableButtonClicked()
{
	if (DataTablePropertyHandle.IsValid())
	{
		UObject* SourceDataTable = nullptr;
		if (DataTablePropertyHandle->GetValue(SourceDataTable) == FPropertyAccess::Success)
		{
			TArray<FAssetData> Assets;
			Assets.Add(SourceDataTable);
			GEditor->SyncBrowserToObjects(Assets);
		}
	}	
}

bool FStevesFixedDataTableCustomisationLayout::GetCurrentValue(UDataTable*& OutDataTable, FName& OutName) const
{
	if (RowNamePropertyHandle.IsValid() && RowNamePropertyHandle->IsValidHandle() && DataTablePropertyHandle.IsValid() && DataTablePropertyHandle->IsValidHandle())
	{
		// If either handle is multiple value or failure, fail
		UObject* SourceDataTable = nullptr;
		if (DataTablePropertyHandle->GetValue(SourceDataTable) == FPropertyAccess::Success)
		{
			OutDataTable = Cast<UDataTable>(SourceDataTable);

			if (RowNamePropertyHandle->GetValue(OutName) == FPropertyAccess::Success)
			{
				return true;
			}
		}
	}
	return false;
}

void FStevesFixedDataTableCustomisationLayout::OnSearchForReferences()
{
	UDataTable* DataTable;
	FName RowName;

	if (GetCurrentValue(DataTable, RowName) && DataTable)
	{
		TArray<FAssetIdentifier> AssetIdentifiers;
		AssetIdentifiers.Add(FAssetIdentifier(DataTable, RowName));

		FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers, FReferenceViewerParams());
	}
}

FString FStevesFixedDataTableCustomisationLayout::OnGetRowValueString() const
{
	if (!RowNamePropertyHandle.IsValid() || !RowNamePropertyHandle->IsValidHandle())
	{
		return FString();
	}

	FName RowNameValue;
	const FPropertyAccess::Result RowResult = RowNamePropertyHandle->GetValue(RowNameValue);
	if (RowResult == FPropertyAccess::Success)
	{
		if (RowNameValue.IsNone())
		{
			return LOCTEXT("DataTable_None", "None").ToString();
		}
		return RowNameValue.ToString();
	}
	else if (RowResult == FPropertyAccess::Fail)
	{
		return LOCTEXT("DataTable_None", "None").ToString();
	}
	else
	{
		return LOCTEXT("MultipleValues", "Multiple Values").ToString();
	}
}

void FStevesFixedDataTableCustomisationLayout::OnGetRowStrings(TArray< TSharedPtr<FString> >& OutStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems) const
{
	UDataTable* DataTable = nullptr;
	FName IgnoredRowName;

	// Ignore return value as we will show rows if table is the same but row names are multiple values
	GetCurrentValue(DataTable, IgnoredRowName);

	TArray<FName> AllRowNames;
	if (DataTable != nullptr)
	{
		for (TMap<FName, uint8*>::TConstIterator Iterator(DataTable->GetRowMap()); Iterator; ++Iterator)
		{
			AllRowNames.Add(Iterator.Key());
		}

		// Sort the names alphabetically.
		AllRowNames.Sort(FNameLexicalLess());
	}

	for (const FName& RowName : AllRowNames)
	{
		OutStrings.Add(MakeShared<FString>(RowName.ToString()));
		OutRestrictedItems.Add(false);
	}
}


#undef LOCTEXT_NAMESPACE

