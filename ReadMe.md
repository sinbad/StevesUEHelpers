# Steve's UE Helper Plugin Library

## Introduction

This is a helper plugin library for [Unreal Engine 4](https://www.unrealengine.com) 
which makes a bunch of things better:

* [New Widgets](doc/Widgets.md)
  * Focusable buttons, menu stacks, control prompts and more
* [Input events](doc/Input.md)
  * Reliable notification when the player changes input method

:heart: **[Support my work on Patreon!](https://www.patreon.com/stevestreeting)**

## Installing this plugin

### Cloning

The best way is to clone this repository as a submodule; that way you can contribute
pull requests if you want. The project should be placed in your project's Plugins folder.

```
> cd YourProject
> git submodule add https://github.com/sinbad/StevesUEHelpers Plugins/StevesUEHelpers
> git add ../.gitmodules
> git commit
```

Alternatively you can download the ZIP of this repo and place it in 
`YourProject/Plugins/StevesUEHelpers`.

### Referencing in C++

Edit YourProject.Build.cs and do something similar to this:

```csharp
public class YourProject : ModuleRules
{
	private string PluginsPath
	{
		get { return Path.GetFullPath( Path.Combine( ModuleDirectory, "../../Plugins/" ) ); }
	}

	public YourProject(ReadOnlyTargetRules Target) : base(Target)
	{
        // Your existing rules
        // ...


		AddStevesUEHelpers();
	}

	protected void AddStevesUEHelpers() {
		// Linker
		PrivateDependencyModuleNames.AddRange(new string[] { "StevesUEHelpers" });
		// Headers
		PublicIncludePaths.Add(Path.Combine( PluginsPath, "StevesUEHelpers", "Source", "StevesUEHelpers", "Public"));
	}

```

You can use most of the features without doing anything else, but certain features
require some additional setup, see below.

## Game instance subsystem

In order to track stateful things like the current input mode for each player, 
there is a custom `GameInstanceSubsystem` called `StevesGameSubsystem`, which 
you can tap into anywhere in Blueprints by searching for it:

![Game Instance Subsystem](./Resources/gameinstance.png)

Once you have access to this you can do things like get the latest input device used:

![Game Instance Subsystem](./Resources/lastinput.png)

For more details, see the [Input section](doc/Input.md).

# License

The MIT License (MIT)
Copyright © 2020 Steve Streeting

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
