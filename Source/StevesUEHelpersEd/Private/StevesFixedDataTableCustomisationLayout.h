#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class IPropertyHandle;
class SToolTip;
class UDataTable;
class UScriptStruct;
struct FAssetData;

/// Drop-down for data table row name when the table itself is fixed in meta tags
class FStevesFixedDataTableCustomisationLayout : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() 
	{
		return MakeShareable( new FStevesFixedDataTableCustomisationLayout );
	}

	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	// Not needed, but must be implemented
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override {}

protected:
	bool GetCurrentValue(UDataTable*& OutDataTable, FName& OutName) const;
	void OnSearchForReferences();
	void OnGetRowStrings(TArray< TSharedPtr<FString> >& OutStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems) const;
	FString OnGetRowValueString() const;
	void BrowseTableButtonClicked();

	TSharedPtr<IPropertyHandle> DataTablePropertyHandle;
	TSharedPtr<IPropertyHandle> RowNamePropertyHandle;

};
