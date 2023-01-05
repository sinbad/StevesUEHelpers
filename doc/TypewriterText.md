# Typewriter Rich Text Widget

This base User Widget provides you with a rich text box that plays text like
a typewriter, one character at a time, with a configurable speed. You can 
skip to the end of the text if needed.

![Typewriter Text](../Resources/typewriterexample.gif)

## Notable features

* Pre-calculates line breaks so that the text doesn't start to play a word near
  the end of the line then "jump down", it always knows where to start
* Pre-calculates the desired height of the text so that if embedded in a flexible
  height container, the height is correct before anything is played
* Supports decorators like inline images
* Respects explicit line breaks in your text
* Pauses on sentence terminators within a multi-sentence block (configurable)

## Usage

Due to a quirk of the API used for sizing, this isn't a straight `URichTextBlock`
subclass you can drop straight in. Instead, you need to:

1. Create a Widget Blueprint, subclassed from `UTypewriterTextWidget`
2. Place within it a `URichTextBlockForTypewriter`, named `LineText` (for binding)
3. Style the nested rich text box the way you want
4. Then, in your other Widget Blueprints, make use of this `UTypewriterTextWidget` 
   subclass in place of a rich text box
5. Call the `Play Line` and `Skip To Line End` functions instead of `Set Text`
   to make use of the typewriter effect.
6. Listen to the `On Typewriter Line Finished` event to know when the typewriter
   effect has finished, or the line has been skipped to the end.


## Credits

This was derived from [Sam Bloomberg's
work](https://github.com/redxdev/UnrealRichTextDialogueBox) with some
enhancements / adjustments. Both works are released under the MIT license.