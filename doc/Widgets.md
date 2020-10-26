# Widgets


Several custom widgets are supplied to assist with some common challenges:

* [OptionWidgetBase](OptionWidget.md)

  This widget base class adds "option switch" functionality, allowing a user to 
  select one of a number of options by moving through them in a linear list.
  It handles both mouse and gamepad by automatically switching styles between
  separate clickable arrows for mouse, and a unified left/right rocker style
  for gamepads. Styleable in Blueprint subclasses.

* [InputImage](InputImage.md)

  This custom Image widget takes an Action or Axis name and will automatically
  display the image for an associated bound control, based on the currently
  active input method. Dynamically switches as input method changes.

* [FocusableButton](FocusableButton.md)

  A refined Button widget which raises focus events you can listen to, and
  which can apply the "Hovered" style to itself when focused (very important
  for gamepad navigation).

* [FocusablePanel](FocusablePanel.md)

  A Panel widget which can make sure that something is selected when a
  gamepad is in use, and resists loss of focus. Has a default focus widget,
  and also remembers the last focus widget if you switch away & back
  without destroying it.

* [MenuBase](Menus.md)

  A specialised [FocusablePanel](FocusablePanel.md) which adds the ability
  to be part of a contextual [MenuStack](Menus.md), and which 
  as it becomes the top of the stack can automatically grab focus, change game 
  pause state, alter input modes, and change the mouse pointer visibility 
  (all individually optional).

* [MenuStack](Menus.md)

  A container for and stack of [MenuBase](Menus.md) instances, making it
  easy to create multi-level on-screen menus with a simple "back" navigation.


