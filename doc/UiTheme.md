# UiTheme

Some features of this plugin such as InputImage need a `UUiTheme` asset, which 
is just a Data Asset based on the `UUiTheme` class which references other 
resources like button images. There is one in the 
[Examples project](https://github.com/sinbad/StevesUEExamples) as reference, 
under Content/Data/UI.

## Making UiTheme a Primary Asset

UiThemes are a new kind of primary asset, loaded from code at runtime. They're not directly
linked in a Blueprint, so to make sure you can load them, and also that they
are packaged in your project, you need to register them with Unreal. 

There are a couple of ways to do this:

1. Directly reference the UiTheme type
2. Use Primary Asset Labels to tag things indirectly

I prefer the latter, because you only have to change your project settings once, 
and from then on you just need to create Primary Asset Labels whenever you
want something else that's loaded dynamically at runtime from code like this.

### Set up your project to use Primary Asset Labels

1. Edit > Project Settings
1. Click "Asset Manager" in the tree
1. Expand "Primary Asset Types To Scan"
1. Add a new entry to this array:
   1. Primary Asset Type = PrimaryAssetLabel
   1. Asset Base Class = PrimaryAssetLabel
   1. Has Blueprint Classes = false
   1. Is Editor Only = false (important! packaging won't work otherwise)
   1. Directories = /Game  (this just means you can use PrimaryAssetLabels anywhere)
   1. Rules > Apply Recursively = true
   1. Cook Rule = Unknown (this just means you can choose in each label)


## Create a UiTheme

1. Click Add New > Miscellaneous > Data Asset
1. Select "UiTheme" as the class
1. Save the new asset with your chosen name

## Setting the default theme

To avoid having to configure the theme for every usage, this library has a
default theme, which is configured in Steve's Game Subsystem. The best way
to set this is by editing your DefaultGame.ini:

```
[/Script/StevesUEHelpers.StevesGameSubsystem]
DefaultUiThemePath="/Game/Path/To/DefaultUiTheme.DefaultUiTheme"
```

That ".UiTheme" postfix is required, it's how UE knows what the type is.

You need to **restart the editor** after making this change.

## Create the PrimaryAssetLabel

We need to tag this UiTheme as relevant for runtime use and packaging using the
PrimaryAssetLabel, since it won't be directly referenced by another asset:

1. In the same directory as the uiTheme, click Add New > Miscellaneous > Data Asset
1. Select "Primary Asset Label" as the class
1. Name it however you like
1. Double-click to edit
1. Under "Explicit Assets", add an entry and pick the UiTheme you created above
1. Save the changes


## Create button sprite data

The UiTheme wants to reference DataTables which contain links between the input
keys and button sprites. So the first job is to create the button sprites.

The [Examples project](https://github.com/sinbad/StevesUEExamples) includes some 
button sprites already; contained in a packed sprite sheet. I created these using 
TexturePacker and imported into UE which created the sprites, but you can create 
them however you like. However, we do require sprites rather than plain textures.

This means you must have the Paper2D plugin enabled in your project.

## Create DataTables linking inputs to button sprites 

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

Again see the [Examples project](https://github.com/sinbad/StevesUEExamples) for
a concrete example, in the Content/Data/UI folder.
