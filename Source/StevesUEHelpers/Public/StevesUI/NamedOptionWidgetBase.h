// 

#pragma once

#include "CoreMinimal.h"
#include "OptionWidgetBase.h"
#include "NamedOptionWidgetBase.generated.h"

/**
 * Subclass of UOptionWidget base to provide convenient access via FNames, on top of the regular integers.
 * Unlike UOptionWidgetBase, this widget can only be populated in code, you can't set up the options
 * and the default selection in the designer.
 */
UCLASS()
class STEVESUEHELPERS_API UNamedOptionWidgetBase : public UOptionWidgetBase
{
	GENERATED_BODY()
protected:
	TMap<FName, int> NameIndex;
	FName SelectedName = NAME_None;

public:
	virtual void ClearOptions() override;
	/**
	 * @brief Adds a new named option
	 * @param Name The name to identify the option by
	 * @param Option The text for the new option
	 * @return The index for the new option
	 */
	UFUNCTION(BlueprintCallable, Category="NamedOptionWidget")
	virtual int AddNamedOption(FName Name, FText Option);


	virtual void SetOptions(const TArray<FText>& Options, int NewSelectedIndex = 0) override;
	/**
     * @brief Sets all of the named options available for this control
     * @param NewOptions All options to be available, indexed by name
     * @param NewSelectedName Which of the options to select by default
     */
    UFUNCTION(BlueprintCallable, Category="NamedOptionWidget")
    virtual void SetNamedOptions(const TMap<FName, FText>& NewOptions, FName NewSelectedName = NAME_None);
    
    UFUNCTION(BlueprintPure, Category="NamedOptionWidget")
    virtual FName GetSelectedName() const;

	/// Get the index in this option list corresponding to a name
    UFUNCTION(BlueprintCallable, Category="NamedOptionWidget")
	virtual int GetIndexForName(FName Name);

	/// Get the name in this option list corresponding to an index
	UFUNCTION(BlueprintCallable, Category="NamedOptionWidget")
	virtual FName GetNameForIndex(int Index);

	virtual void SetSelectedIndex(int NewIndex) override;
	/**
     * @brief Change the selected option by name
     * @param Name The name of the new option to set, can be None for no selection
     */
    UFUNCTION(BlueprintCallable, Category="NamedOptionWidget")
    virtual void SetSelectedName(FName Name);
};
