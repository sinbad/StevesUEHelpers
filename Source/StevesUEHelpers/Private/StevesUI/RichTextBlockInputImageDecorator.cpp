#include "StevesUI/RichTextBlockInputImageDecorator.h"


#include "StevesHelperCommon.h"
#include "StevesUEHelpers.h"
#include "Fonts/FontMeasure.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"


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
    /// Player index, if binding type is action or axis
    int PlayerIndex;
    /// Where there are multiple mappings, which to prefer 
    EInputImageDevicePreference DevicePreference = EInputImageDevicePreference::Auto;

    /// Initial Sprite to use
    UPaperSprite* InitialSprite;
    /// Parent decorator, for looking up things later
    URichTextBlockInputImageDecorator* Decorator;
};
// Basically the same as SRichInlineImage but I can't re-use that since private
class SRichInlineInputImage : public SCompoundWidget
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
    /// Player index, if binding type is action or axis
    int PlayerIndex = 0;
    /// Where there are multiple mappings, which to prefer 
    EInputImageDevicePreference DevicePreference = EInputImageDevicePreference::Auto;
    /// Parent decorator, for looking up things later
    URichTextBlockInputImageDecorator* Decorator = nullptr;

    FSlateBrush Brush;
    float TimeUntilNextSpriteCheck = 0;
    uint16 MaxCharHeight = 0;
    TOptional<int32> RequestedWidth;
    TOptional<int32> RequestedHeight;
    SBox* ContainerBox = nullptr;

public:
    SLATE_BEGIN_ARGS(SRichInlineInputImage)
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
        PlayerIndex = InParams.PlayerIndex;
        Decorator = InParams.Decorator;
        RequestedWidth = Width;
        RequestedHeight = Height;

        // Sadly, we cannot hook into the events needed to update based on input changes here
        // All attempts to use GetStevesGameSubsystem() fail because the world pointer
        // doesn't work, I think perhaps because this Slate Construct call is in another thread which pre-dates it.
        // We will need to do the work to update the brush from the main thread later

        // We can use static methods though
        if (IsValid(InParams.InitialSprite))
            UStevesGameSubsystem::SetBrushFromAtlas(&Brush, InParams.InitialSprite, true);
        TimeUntilNextSpriteCheck = 0.25f;

        const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        MaxCharHeight = FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f);
        float IconHeight = FMath::Min(static_cast<float>(MaxCharHeight), Brush.ImageSize.Y);
        if (Height.IsSet())
        {
            IconHeight = Height.GetValue();
        }

        float IconWidth = Brush.ImageSize.X * (IconHeight / Brush.ImageSize.Y) ;
        if (Width.IsSet())
        {
            IconWidth = Width.GetValue();
        }

        ChildSlot
        [
            SNew(SBox)
            .HeightOverride(IconHeight)
            .WidthOverride(IconWidth)
            [
                SNew(SScaleBox)
                .Stretch(Stretch)
                .StretchDirection(EStretchDirection::DownOnly)
                .VAlign(VAlign_Center)
                [
                    SNew(SImage)
                    .Image(&Brush)
                ]
            ]
        ];
    }


    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
    {
        SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

        // I would prefer to hook into the events here but there is NO safe teardown in these RichText decorators
        // with which to unsub from events, so we're down to brute forcing this in Tick() le sigh
        // At least limit the frequency and only change as needed
        TimeUntilNextSpriteCheck -= InDeltaTime;
        if (TimeUntilNextSpriteCheck <= 0)
        {
            auto GS = GetStevesGameSubsystem(Decorator->GetWorld());
            if (GS)
            {
                // Can only support default theme, no way to edit theme in decorator config 
                UPaperSprite* Sprite = nullptr;
                if (BindingType == EInputBindingType::EnhancedInputAction && !InputAction.IsNull())
                {
                    if (auto IA = InputAction.LoadSynchronous())
                    {
                        auto PC = UGameplayStatics::GetPlayerController(Decorator->GetWorld(), PlayerIndex);
                        Sprite = GS->GetInputImageSpriteFromEnhancedInputAction(IA, DevicePreference, PlayerIndex, PC);
                    }
                }
                else
                {
                    Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, DevicePreference, PlayerIndex);    
                }
                if (Sprite && Brush.GetResourceObject() != Sprite)
                {
                    UStevesGameSubsystem::SetBrushFromAtlas(&Brush, Sprite, true);

                    // Deal with aspect ratio changes
                    TSharedPtr<SWidget> Widget = ChildSlot.GetWidget();
                    SBox* Box = static_cast<SBox*>(Widget.Get());
                    float IconHeight = FMath::Min(static_cast<float>(MaxCharHeight), Brush.ImageSize.Y);
                    if (RequestedHeight.IsSet())
                    {
                        IconHeight = RequestedHeight.GetValue();
                    }
                    
                    float IconWidth = Brush.ImageSize.X * (IconHeight / Brush.ImageSize.Y) ;
                    if (RequestedWidth.IsSet())
                    {
                        IconWidth = RequestedWidth.GetValue();
                    }

                    Box->SetWidthOverride(IconWidth);
                    Box->SetHeightOverride(IconHeight);
                    
                }
                
            }
            TimeUntilNextSpriteCheck = 0.5f;
        }
    }
};

// Again, wish I could just subclass FRichInlineImage here, le sigh
class FRichInlineInputImage : public FRichTextDecorator
{
public:
    FRichInlineInputImage(URichTextBlock* InOwner, URichTextBlockInputImageDecorator* InDecorator)
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
                    Params.InitialSprite = GS->GetInputImageSpriteFromEnhancedInputAction(IA, Params.DevicePreference, Params.PlayerIndex, PC);
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
        return SNew(SRichInlineInputImage, Params, TextStyle, Width, Height, Stretch);
    }

private:
    URichTextBlockInputImageDecorator* Decorator;
};

TSharedPtr<ITextDecorator> URichTextBlockInputImageDecorator::CreateDecorator(URichTextBlock* InOwner)
{
    return MakeShareable(new FRichInlineInputImage(InOwner, this));
}
