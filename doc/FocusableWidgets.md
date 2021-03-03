# Focusable Widgets

This library includes some subclasses of core widgets to better handle focus, 
including better highlight rendering when the button has the keyboard / gamepad focus,
events you can listen in to when focus changes, and more robust navigation.

Currently they include:

* FocusableButton (particularly useful in [Menus](Menus.md), but can be used anywhere)
* FocusableCheckBox


## Important: Disabling default focus rendering

Unreal makes a token effort at providing a way to render which of your controls
has the keyboard / gamepad focus, but it's rubbish. It's a grey dotted line
that looks awful. No-one wants that.

The widgets in this library provide a much nicer default focus
option (using the Hovered style), and also provides you focus events you can
use to do something more advanced if you want. So we have no need for Unreal's
dotted line nonsense.

1. Open Project Settings
1. Find the Focus section
1. Change Render Focus Rule to Never

All better. 😉

## Better Focus Highlighting

Focusable widgets can highlight in the same way as the "Hover"
style, thereby unifying how your button looks whether the player mouses over
it, or selects it with keyboard / gamepad. 

All you need to do to benefit from this is:

1. Enable "Use Hover Style When Focussed" in the inspector
2. Define the Hovered style under Appearance

## Changing focus on Hover

There's also a "Take Focus On Hover" option in the inspector (default true).
This ensures that if one widget has the focus currently, from keyboard or gamepad 
navigation, and you hover over another widget with the mouse, that other widget
will lose focus in favour of the hovered widget. 

The main benefit of this is that otherwise you can have 2 widgets highlighted,
one via the old focus and one where the mouse is hovering, which is confusing!

## Focus Events

Focusable widgets also have 2 new events exposed, "On Focus Received" and "On Focus Lost".
You can use this to track which widget currently has the focus, which can be
useful if for example you want to use something else to indicate the current 
focus, like an icon or indicator that isn't part of the widget itself.





