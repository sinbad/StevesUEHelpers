# FocusablePanel

You can use this as a base class for your widgets instead of User Widget,
and get the following behaviour:

## Automatic focus

Like [Focusable User Widget](FocusableUserWidget.md) (it's a subclass) this
panel can opt in to automatically getting focus when it's opened, and getting it
back when others lose focus.

## Initial focus

Set the Initial Focus Widget Name to the name of a focusable widget in this panel.
When the panel os attached to the viewport, this widget will be given the focus.

## Remembering focus

This widget provides `SavePreviousFocus` and `RestorePreviousFocus` methods 
which can save / restore focus from one of its children as needed. They are 
not called by default but are used in [Menus](Menus.md).