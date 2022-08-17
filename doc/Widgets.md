# Widgets


Several custom widgets are supplied to assist with some common challenges:

* [Option Widget](OptionWidget.md)

  A widget that adds "option switch" functionality, allowing a user to 
  select one of a number of options by moving through them in a linear list.
  It handles both mouse and gamepad by automatically switching styles between
  separate clickable arrows for mouse, and a unified left/right rocker style
  for gamepads. Styleable in Blueprint subclasses.

* [Input Prompts](InputImage.md)

  A custom Image widget that takes an Action or Axis name and will automatically
  display the image for an associated bound control, based on the currently
  active input method. Dynamically switches as input method changes.

* [Rich Text Input Decorator](RichTextInputDecorator.md)

  A decorator for Rich Text controls that allows you to use markup like
  `<input action="SomeGameAction"/>` in your text to inline an image of 
  the input control which is bound to that input action right now. 
  Again this dynamically switches as input method changes.

* [Typewriter Rich Text Widget](TypewriterText.md)

  A widget that allows you to use the "typewriter effect" to display text
  (displaying one character at a time). Pre-calculates line breaks so text
  doesn't try to fit in then jump down, and pre-calculates desired height so
  your flexible boxes are always the right size *before* the text is played.

* [Menu System](Menus.md)

  A couple of classes to make it  easy to create multi-level on-screen menus 
  with a simple "back" navigation. They're compatible with mouse and gamepad/keyboard 
  navigation and deal with a bunch of edge cases around switching between those. 

  Menus can change the game pause state, alter input modes and change the
  mouse pointer visibility, on both opening and closing, to make it easier
  to implement this common behaviour.

* [Focusable Button](FocusableWidgets.md)

  A refined Button widget which raises focus events you can listen to, and
  which can apply the "Hovered" style to itself when focused (very important
  for gamepad navigation).

* [Focusable CheckBox](FocusableWidgets.md)

  An improved CheckBox widget which raises focus events you can listen to, 
  is navigable by gamepad / keyboard, and which can apply the "Hovered" 
  style to itself when focused.

* [Focusable User Widget](FocusableUserWidget.md)

  A base class to use as an alternative to plain UserWidget if you want this
  widget to have the option to automatically gain focus. This is a good base class
  for dialogs or other systems which are navigable by gamepad or keyboard.

* [Focusable Panel](FocusablePanel.md)

  A Panel widget which can make sure that something is selected when a
  gamepad is in use, and resists loss of focus. Has a default focus widget,
  and also remembers the last focus widget if you switch away & back
  without destroying it.

