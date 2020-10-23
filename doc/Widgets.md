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

* [MenuBase](MenuBase.md)

  A specialised [FocusablePanel](FocusablePanel.md) which adds the ability
  to be part of a contextual [MenuStack](MenuStack.md), and which 
  as it becomes the top of the stack can automatically grab focus, change game 
  pause state, alter input modes, and change the mouse pointer visibility 
  (all individually optional).

* [MenuStack](MenuStack.md)

  A container for and stack of [MenuBase](MenuBase.md) instances, making it
  easy to create multi-level on-screen menus with a simple "back" navigation.


# Additional Configuration

## UiTheme

Some features of this plugin such as InputImage need a `UUiTheme` asset, which 
is just a Data Asset based on the `UUiTheme` class which references other 
resources like button images. There is one in the Examples project as reference.

### Create a UiTheme:


1. Click Add New > Miscellaneous > Data Asset
1. Select "UiTheme" as the class
1. Save the new asset with your chosen name

### Create a Primary Asset Label

UiThemes are a new kind of primary asset, loaded at runtime. To ensure this
asset is included when packaging, create a Primary Asset Label in the same folder:

1. Click Add New > Miscellaneous > Data Asset
1. Select "Primary Asset Label" as the class
1. Name it however you like
1. Double-click to edit
1. Under "Explicit Assets", add an entry and pick the UiTheme you created above
1. Save the changes

This just ensures that the packaging system knows to include your UiTheme, since
it won't be directly referenced by any other primary asset.

### Create button sprite data

The UiTheme wants to reference DataTables which contain links between the input
keys and button sprites. So the first job is to create the button sprites.

The Example project includes some button sprites already; contained in a packed
sprite sheet. I created these using TexturePacker and imported into UE which created
the sprites, but you can create them however you like. However, we do require sprites
rather than plain textures.

This means you must have the Paper2D plugin enabled in your project.

### Linking input keys to button sprites 
Once you have a set of button sprites, you need to create DataTables which map
input FKeys (which can be keys, or mouse buttons, or gamepad buttons / sticks)
to these sprites, so that for example InputImage can be told to display the action 
"Fire", and then display either say the left mouse button or a gamepad trigger
depending on what's being used.

Personally I did this using a CSV file for ease of use, for example:

```csv
Name,Key,Sprite
Gamepad_LeftX,Gamepad_LeftX,"PaperSprite'/Game/Textures/UI/Sprites/Frames/XboxOne_Left_Stick'"
Gamepad_FaceButton_Bottom,Gamepad_FaceButton_Bottom,"PaperSprite'/Game/Textures/UI/Sprites/Frames/XboxOne_A'"
```

You should import this as a DataTable based on the KeySprite type. A separate
one is needed for keyboard / mouse and gamepad. Once you've created them, or
copied the ones from the examples, you should update the UiTheme asset you 
created to point at these data tables.
