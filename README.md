# Plant care

A tile for one plant. It shows when the plant was last watered and last fed, each in a row with a dot that goes from
green to amber to red as the plant gets due. A tap on a row on the screen says "done today".

## Set up

1. Add the plugin to a screen in Tessera. The screen builds once.
2. In Layout, place the **Plant** tile from the library's Plugins group.
3. In the inspector, give the plant a name and say how often it is watered and fed (days). The dot is green until that
   many days have passed, amber for about half as long again, then red.

## On the screen

- Tap the **upper row** when you have watered the plant, the **lower row** when you have fed it. The day is today.
- Tapping the name does nothing, so a stray touch cannot change a date.

## Good to know

- The two dates are entities of the screen in Home Assistant, **Plant last watered** and **Plant last fertilized**
  (entities of type date). Set an earlier day there when you forgot to tap. They are kept on the screen and survive a
  restart.
- A screen has one plant: two plant tiles on one screen show the same two dates.
- The tile counts on the screen's own clock and changes at midnight. Until Home Assistant has set the clock, the rows
  stay empty.
