# Flood Fill

This is a simple flood fill game for the Arduboy. 

Starting from the top-left tile, the goal is to flood the entire board until there is only one pattern.

There are 999 levels. Each level has a limit of 20 moves.

## Gameplay

![Gameplay](/img/ArduboyRecording.gif)

## How to Play


### Title Screen

  - Press UP or DOWN to change the level by one.
  - Press LEFT or RIGHT to change the level by 10.
  - Press A to start.

### Game Screen

  - Press UP, DOWN, LEFT or RIGHT to select a tile to flood. 
  - Press A to flood the board (starting in the upper left corner) with the tile you selected.
  - Press B to return to the title screen.

## Programming Notes

  - This has only been tested on an Arduboy 1.0.
  - This game saves level selection to EEPROM.
