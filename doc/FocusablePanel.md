# FocusablePanel

You can use this as a base class for your widgets instead of User Widget,
and get the following behaviour:

## Initial focus

Set the Initial Focus Widget Name to the name of a focusable widget in this panel.
When the panel os attached to the viewport, this widget will be given the focus.

## Remembering focus

This widget provides `SavePreviousFocus` and `RestorePreviousFocus` methods 
which can save / restore focus as needed. They are not called by default but 
are used in [Menus](Menus.md).