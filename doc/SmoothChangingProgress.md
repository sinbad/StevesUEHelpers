# Smooth Changing Progress Bar

This is a fairly simple extension to Progress Bar to allow it to change smoothly
rather than jumping.

You configure this as follows:

* Set `PercentChangeSpeed` to the amount of percent change per second
* Optionally set `PercentChangeFrequency`; at 0, it updates every frame, otherwise
  it can update less frequently by setting this to a number of seconds
* Call `SetPercentSmoothly` instead of `SetPercent`
   * (`SetPercent` is not virtual in `UProgressBar` so we cannot override that. This
      also means that if you want to interrupt the smooth change, you need to call
      `StopSmoothPercentChange`)


## See Also

 * [Widgets](Widgets.md)