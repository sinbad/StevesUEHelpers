// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#include "StevesUI/RichTextBlockInputImageDecorator.h"


#include "StevesHelperCommon.h"
#include "StevesUEHelpers.h"
#include "Fonts/FontMeasure.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"


// Slate SNew only supports 5 custom arguments so we need to batch things up
struct FRichTextInputImageParams
{
    /// What type of an input binding this image should look up
    EInputBindingType BindingType;
    /// If BindingType is Action/Axis, the name of it 
    FName ActionOrAxisName;
    /// If BindingType is Key, the key 
    FKey Key;
    /// If binding type is EnhancedInputAction, a reference to an enhanced input action
    TSoftObjectPtr<UInputAction> InputAction;
	/// Whether to search inactive action mappings as well as active ones
	bool bSearchInactiveActionMappings = false;
	/// The max number of mappings to display instead of just the first. Useful for actions which have multiple
	/// keys bound to different axes
	int MaxNumMappings = 1;
    /// Player index, if binding type is action or axis
    int PlayerIndex;
    /// Where there are multiple mappings, which to prefer 
    EInputImageDevicePreference DevicePreference = EInputImageDevicePreference::Auto;

    /// Initial Sprite to use
    UPaperSprite* InitialSprite;
    /// Parent decorator, for looking up things later
    URichTextBlockInputImageDecorator* Decorator;
};
// We're a horizontal box so we can display multiple images if we want (e.g. WSAD is 4 input images for one 2D axis)
class SStevesRichInlineInputImage : public SHorizontalBox
{
protected:
    /// What type of an input binding this image should look up
    EInputBindingType BindingType = EInputBindingType::Key;
    /// If BindingType is Action/Axis, the name of it 
    FName ActionOrAxisName;
    /// If BindingType is Key, the key 
    FKey Key;
    /// If binding type is EnhancedInputAction, a reference to an enhanced input action
    TSoftObjectPtr<UInputAction> InputAction;
	/// Whether to search inactive action mappings as well as active ones
	bool bSearchInactiveActionMappings = false;
	/// The max number of mappings to display instead of just the first. Useful for actions which have multiple
	/// keys bound to different axes
	int MaxNumMappings = 1;
	/// Player index, if binding type is action or axis
    int PlayerIndex = 0;
    /// Where there are multiple mappings, which to prefer 
    EInputImageDevicePreference DevicePreference = EInputImageDevicePreference::Auto;
    /// Parent decorator, for looking up things later
    URichTextBlockInputImageDecorator* Decorator = nullptr;

    float TimeUntilNextSpriteCheck = 0;
    uint16 MaxCharHeight = 0;
    TOptional<int32> RequestedWidth;
    TOptional<int32> RequestedHeight;
	
	TArray<TSharedPtr<SBox>> ImageBoxes;
	TArray<FSlateBrush> Brushes;
	int CurrentImageCount = 0;

public:
    SLATE_BEGIN_ARGS(SStevesRichInlineInputImage)
    {}
    SLATE_END_ARGS()

public:

    void Construct(const FArguments& InArgs, FRichTextInputImageParams InParams,
                   const FTextBlockStyle& TextStyle, TOptional<int32> Width, TOptional<int32> Height, EStretch::Type Stretch)
    {
        BindingType = InParams.BindingType;
        ActionOrAxisName = InParams.ActionOrAxisName;
        DevicePreference = InParams.DevicePreference;
        Key = InParams.Key;
        InputAction = InParams.InputAction;
    	bSearchInactiveActionMappings = InParams.bSearchInactiveActionMappings;
    	MaxNumMappings = InParams.MaxNumMappings;
        PlayerIndex = InParams.PlayerIndex;
        Decorator = InParams.Decorator;
        RequestedWidth = Width;
        RequestedHeight = Height;
    	CurrentImageCount = 0;
    	
    	// Horizontal box disables the default tick
    	SetCanTick(true);

        // Sadly, we cannot hook into the events needed to update based on input changes here
        // All attempts to use GetStevesGameSubsystem() fail because the world pointer
        // doesn't work, I think perhaps because this Slate Construct call is in another thread which pre-dates it.
        // We will need to do the work to update the brush from the main thread later
    	
    	ImageBoxes.SetNum(MaxNumMappings);
    	Brushes.SetNum(MaxNumMappings);

    	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    	MaxCharHeight = FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f);

    	float IconHeight = 32;
    	float IconWidth = 32;
        CurrentImageCount = 1; // always show one thing so not invisible in design mode
        if (IsValid(InParams.InitialSprite))
        {
	        UStevesGameSubsystem::SetBrushFromAtlas(&Brushes[0], InParams.InitialSprite, true);

        	IconHeight = FMath::Min(static_cast<float>(MaxCharHeight), Brushes[0].ImageSize.Y);
        	if (Height.IsSet())
        	{
        		IconHeight = Height.GetValue();
        	}

        	IconWidth = Brushes[0].ImageSize.X * (IconHeight / Brushes[0].ImageSize.Y) ;
        	if (Width.IsSet())
        	{
        		IconWidth = Width.GetValue();
        	}
        	
        }
        TimeUntilNextSpriteCheck = 0.05f;


    	for (int i = 0; i < MaxNumMappings; ++i)
    	{
    		AddSlot()
			[
				SAssignNew(ImageBoxes[i], SBox)
				.HeightOverride(IconHeight)
				.WidthOverride(IconWidth)
				.Visibility(i < CurrentImageCount ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed)
				[
					SNew(SScaleBox)
					.Stretch(Stretch)
					.StretchDirection(EStretchDirection::DownOnly)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(&Brushes[i])
					]
				]
			];
    	}
    	
    	// This might not be right but try anyway
    	UpdateImages();
    }
	
	void UpdateImages()
    {
	    auto GS = GetStevesGameSubsystem(Decorator->GetWorld());
	    if (GS)
        {
            // Can only support default theme, no way to edit theme in decorator config 
            TArray<UPaperSprite*> Sprites;
            if (BindingType == EInputBindingType::EnhancedInputAction && !InputAction.IsNull())
            {
                if (auto IA = InputAction.LoadSynchronous())
                {
                    auto PC = UGameplayStatics::GetPlayerController(Decorator->GetWorld(), PlayerIndex);
                    GS->GetAllInputImageSpritesFromEnhancedInputAction(Sprites, IA, DevicePreference, PlayerIndex, PC, nullptr, bSearchInactiveActionMappings);
                }
            }
            else
            {
                if (auto Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, DevicePreference, PlayerIndex))
                {
                    Sprites.Add(Sprite);
                }
            }
            const int NewImageCount = FMath::Min(MaxNumMappings, Sprites.Num());
            const int NumToUpdate = FMath::Max(NewImageCount, CurrentImageCount);
            for (int i = 0; i < NumToUpdate; ++i)
            {
            	// Deal with visibility changes
            	if (i >= NewImageCount)
            	{
            		ImageBoxes[i]->SetVisibility(EVisibility::Collapsed);
            	}
            	else if (i >= CurrentImageCount)
            	{
            		ImageBoxes[i]->SetVisibility(EVisibility::SelfHitTestInvisible);
            	}
            	
            	if (i < NewImageCount)
            	{
            		auto Sprite = Sprites[i];
            		if (Sprite && Brushes[i].GetResourceObject() != Sprite)
            		{
            			UStevesGameSubsystem::SetBrushFromAtlas(&Brushes[i], Sprite, true);

            			// Deal with aspect ratio changes
            			float IconHeight = FMath::Min(static_cast<float>(MaxCharHeight), Brushes[i].ImageSize.Y);
            			if (RequestedHeight.IsSet())
            			{
            				IconHeight = RequestedHeight.GetValue();
            			}
                
            			float IconWidth = Brushes[i].ImageSize.X * (IconHeight / Brushes[i].ImageSize.Y) ;
            			if (RequestedWidth.IsSet())
            			{
            				IconWidth = RequestedWidth.GetValue();
            			}

            			ImageBoxes[i]->SetWidthOverride(IconWidth);
            			ImageBoxes[i]->SetHeightOverride(IconHeight);
                
            		}
            	}
            }
            CurrentImageCount = NewImageCount;
            
        }
    }


    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
    {
        SHorizontalBox::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

        // I would prefer to hook into the events here but there is NO safe teardown in these RichText decorators
        // with which to unsub from events, so we're down to brute forcing this in Tick() le sigh
        // At least limit the frequency and only change as needed
        TimeUntilNextSpriteCheck -= InDeltaTime;
        if (TimeUntilNextSpriteCheck <= 0)
        {
        	UpdateImages();
            TimeUntilNextSpriteCheck = 0.5f;
        }
    }
};

// Again, wish I could just subclass FRichInlineImage here, le sigh
class FStevesRichInlineInputImage : public FRichTextDecorator
{
public:
    FStevesRichInlineInputImage(URichTextBlock* InOwner, URichTextBlockInputImageDecorator* InDecorator)
        : FRichTextDecorator(InOwner)
        , Decorator(InDecorator)
    {
    }

    virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
    {
        if (RunParseResult.Name == TEXT("input"))
        {
            return RunParseResult.MetaData.Contains(TEXT("key")) ||
                RunParseResult.MetaData.Contains(TEXT("action")) ||
                RunParseResult.MetaData.Contains(TEXT("axis")) ||
                    RunParseResult.MetaData.Contains(TEXT("eaction"));
        }

        return false;
    }

protected:

    virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const override
    {
        FRichTextInputImageParams Params;
        Params.PlayerIndex = 0;
        Params.BindingType = EInputBindingType::Key;
        Params.Key = EKeys::AnyKey;
        Params.Decorator = Decorator;
        
        auto GS = GetStevesGameSubsystem(Decorator->GetWorld());
        
        if (const FString* PlayerStr = RunInfo.MetaData.Find(TEXT("player")))
        {
            int PTemp;
            Params.PlayerIndex = FDefaultValueHelper::ParseInt(*PlayerStr, PTemp) ? PTemp : 0;            
        }

        if (const FString* KeyStr = RunInfo.MetaData.Find(TEXT("key")))
        {
            Params.BindingType = EInputBindingType::Key;
            Params.Key = FKey(**KeyStr);
        }
        else if (const FString* ActionStr = RunInfo.MetaData.Find(TEXT("action")))
        {
            Params.BindingType = EInputBindingType::Action;
            Params.ActionOrAxisName = **ActionStr;        
        }
        else if (const FString* AxisStr = RunInfo.MetaData.Find(TEXT("axis")))
        {
            Params.BindingType = EInputBindingType::Axis;
            Params.ActionOrAxisName = **AxisStr;        
        }
        else if (const FString* EInputStr = RunInfo.MetaData.Find(TEXT("eaction")))
        {
            Params.BindingType = EInputBindingType::EnhancedInputAction;
            // Try to find the input action
            if (GS)
            {
                Params.InputAction = GS->FindEnhancedInputAction(*EInputStr);
            }
        	
        	if (const FString* InactiveStr = RunInfo.MetaData.Find(TEXT("inactive")))
        	{
        		int BTemp;
        		Params.bSearchInactiveActionMappings = (FDefaultValueHelper::ParseInt(*InactiveStr, BTemp) ? BTemp : 0) > 0;            
        	}

        	if (const FString* NumStr = RunInfo.MetaData.Find(TEXT("maxnum")))
        	{
        		int NTemp;
        		Params.MaxNumMappings = FDefaultValueHelper::ParseInt(*NumStr, NTemp) ? NTemp : 1;            
        	}
        
        }

        if (const FString* PreferStr = RunInfo.MetaData.Find(TEXT("prefer")))
        {
            if (PreferStr->Equals("auto", ESearchCase::IgnoreCase))
            {
                Params.DevicePreference = EInputImageDevicePreference::Auto;
            }
            else if (PreferStr->Equals("gkm", ESearchCase::IgnoreCase))
            {
                Params.DevicePreference = EInputImageDevicePreference::Gamepad_Keyboard_Mouse;
            }
            else if (PreferStr->Equals("gmk", ESearchCase::IgnoreCase))
            {
                Params.DevicePreference = EInputImageDevicePreference::Gamepad_Mouse_Keyboard;
            }
            else if (PreferStr->Equals("gmkbutton", ESearchCase::IgnoreCase))
            {
                Params.DevicePreference = EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Button;
            }
            else if (PreferStr->Equals("gmkaxis", ESearchCase::IgnoreCase))
            {
                Params.DevicePreference = EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Axis;
            }
        }

        // Look up the initial sprite here
        // The Slate widget can't do it in Construct because World pointer doesn't work (thread issues?)
        // Also annoying: can't keep Brush on this class because this method is const. UGH
        if (GS)
        {
            if (Params.BindingType == EInputBindingType::EnhancedInputAction && !Params.InputAction.IsNull())
            {
                if (auto IA = Params.InputAction.LoadSynchronous())
                {
                    auto PC = UGameplayStatics::GetPlayerController(Decorator->GetWorld(), Params.PlayerIndex);
                    Params.InitialSprite = GS->GetInputImageSpriteFromEnhancedInputAction(IA, Params.DevicePreference, Params.PlayerIndex, PC, nullptr, Params.bSearchInactiveActionMappings);
                }
            }
            else
            {
                // Can only support default theme, no way to edit theme in decorator config 
                Params.InitialSprite = GS->GetInputImageSprite(Params.BindingType, Params.ActionOrAxisName, Params.Key, Params.DevicePreference, Params.PlayerIndex);
            }
        }
        else
        {
            // Might be false because this gets executed in the editor too
            // TODO use a placeholder?
            Params.InitialSprite = nullptr;            
        }
    

        // Support the same width/height/stretch overrides as standard rich text images
        TOptional<int32> Width;
        if (const FString* WidthString = RunInfo.MetaData.Find(TEXT("width")))
        {
            int32 WidthTemp;
            Width = FDefaultValueHelper::ParseInt(*WidthString, WidthTemp) ? WidthTemp : TOptional<int32>();
        }
        
        TOptional<int32> Height;
        if (const FString* HeightString = RunInfo.MetaData.Find(TEXT("height")))
        {
            int32 HeightTemp;
            Height = FDefaultValueHelper::ParseInt(*HeightString, HeightTemp) ? HeightTemp : TOptional<int32>();
        }
        
        EStretch::Type Stretch = EStretch::ScaleToFit;
        if (const FString* SstretchString = RunInfo.MetaData.Find(TEXT("stretch")))
        {
            static const UEnum* StretchEnum = StaticEnum<EStretch::Type>();
            int64 StretchValue = StretchEnum->GetValueByNameString(*SstretchString);
            if (StretchValue != INDEX_NONE)
            {
                Stretch = static_cast<EStretch::Type>(StretchValue);
            }
        }

        // SNew only supports 5 custom arguments! Thats why we batch up in struct
        return SNew(SStevesRichInlineInputImage, Params, TextStyle, Width, Height, Stretch);
    }

private:
    URichTextBlockInputImageDecorator* Decorator;
};

TSharedPtr<ITextDecorator> URichTextBlockInputImageDecorator::CreateDecorator(URichTextBlock* InOwner)
{
    return MakeShareable(new FStevesRichInlineInputImage(InOwner, this));
}
