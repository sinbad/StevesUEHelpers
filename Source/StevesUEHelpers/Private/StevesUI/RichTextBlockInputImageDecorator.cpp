#include "StevesUI/RichTextBlockInputImageDecorator.h"


#include "StevesHelperCommon.h"
#include "StevesUEHelpers.h"
#include "Fonts/FontMeasure.h"
#include "Misc/DefaultValueHelper.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"


// Slat SNew only supports 5 custom arguments so we need to batch things up
struct FInputImageParams
{
    /// What type of an input binding this image should look up
    EInputBindingType BindingType;
    /// If BindingType is Action/Axis, the name of it 
    FName ActionOrAxisName;
    /// If BindingType is Key, the key 
    FKey Key;
    /// Player index, if binding type is action or axis
    int PlayerIndex;
    /// Theme, if any
    UUiTheme* CustomTheme;
    UWorld* WorldContext;
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
    /// Player index, if binding type is action or axis
    int PlayerIndex = 0;
    /// Theme, if any
    UUiTheme* CustomTheme = nullptr;
    UWorld* WorldContext = nullptr;

    FSlateBrush Brush;

public:
    SLATE_BEGIN_ARGS(SRichInlineInputImage)
    {}
    SLATE_END_ARGS()

public:

    void Construct(const FArguments& InArgs, FInputImageParams InParams,
        const FTextBlockStyle& TextStyle, TOptional<int32> Width, TOptional<int32> Height, EStretch::Type Stretch)
    {
        BindingType = InParams.BindingType;
        ActionOrAxisName = InParams.ActionOrAxisName;
        Key = InParams.Key;
        PlayerIndex = InParams.PlayerIndex;
        CustomTheme = InParams.CustomTheme;
        WorldContext = InParams.WorldContext;

        const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        float IconHeight = FMath::Min((float)FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f), Brush.ImageSize.Y);
        float IconWidth = IconHeight;

        if (Width.IsSet())
        {
            IconWidth = Width.GetValue();
        }

        if (Height.IsSet())
        {
            IconHeight = Height.GetValue();
        }

        auto GS = GetStevesGameSubsystem(WorldContext);
        if (GS)
        {
            auto Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, PlayerIndex, CustomTheme);
            GS->SetBrushFromAtlas(&Brush, Sprite, true);
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
                RunParseResult.MetaData.Contains(TEXT("axis"));
        }

        return false;
    }

protected:

    virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const override
    {
        FInputImageParams Params;
        Params.PlayerIndex = 0;
        Params.BindingType = EInputBindingType::Key;
        Params.Key = EKeys::AnyKey;
        
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
