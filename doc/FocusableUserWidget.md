# FocusableUserWidget

You can use this as a base class for your widgets instead of User Widget, in 
order to optionally get some automatic  focus behaviour. 

Checking the "Enable Automatic Focus" box opts into this behaviour, and whenever
this widget is added to the viewport it will automatically be given focus *if*
its priority is equal or higher to anything else on screen right now. You set
the "Automatic Focus Priority" member to a value and the highest wins.

When a widget with auto focus enabled is removed from the viewport while it has 
the focus, it triggers the focus system to automatically give focus to the
highest priority widget still in the viewport.

This makes it easier to make UIs with several independent elements, which can
supersede each other, but you need to have a reliable focus chain. By enabling
this option on all your dialogs (it's automatically enabled on [Menus](Menus.md))
and setting priorities accordingly, you can ensure that focus is reliably 
transferred without having to do it manually in all combinations of sequences.

Focus is given by calling the `SetFocusProperly` method. Some subclasses like
[Focusable Panel](FocusablePanel.md) automatically override this to give focus
to a child widget, but you can override it to do whatever you like (default is
just to call `SetFocus` on self). The reason for using `SetFocusProperly` is that `SetFocus`
isn't virtual in the Unreal base classes 🙄

## See Also

* [Focusable Panel](FocusablePanel.md)
* [Menus](Menus.md)