# Jeopardy

* Original Author:  Christian Lange (Christian_Lange@hotmail.com)
* Date:             06. February 2014
* Version:          0.9.6 Stable
* Github:           https://github.com/Volker-Weissmann/jeopardy
* Homepage:         http://ganz-sicher.net/chlange
* License:          New BSD License (3-clause BSD license)

## Description

* Implementation of well known Jeopardy! quiz show in C++ with Qt.

## Features

* up to 9 players
* sound
* colors
* names
* choose own key to answer (Press key on game field to check functionality)
* right click context menu includes
	* random generator to pick random user (Press "r" on game field for same functionality)
	* load/save game state
	* player name and points editor
	* early round ending option
	* round reset
* automated game state backup after each answer 
	* backups can be found in gameStates/backups/
	* backups ordered by round and unix timestamp
* formatted text, sound, images and videos as answer 
	* see answers/README or wiki for further instructions
	* images and videos will be resized if too big
	* sound and videos will stop after 30 seconds (normal answer time)
	* Press Shift to restart sound or video
* double jeopardy questions 
	* see answers/README or wiki for further instructions

## Todo

* video playback on NixOS

## How to build and run

Beware that it will not work if you cd into different directory.

### ArchLinux
* `pacman -S qt5-base qt5-multimedia qt5-xmlpatterns`
* `qmake-qt5`
* `make -j`
* `./jeopardy`

### NixOS
* `nix-shell -p qt5Full`
* `qmake`
* `make -j`
* `./jeopardy`

I could not get video playback to work on NixOS. Images, Sound and the rest works though.

### Windows
* I do not now if it works on Windows, all I could find was [this old link](https://github.com/chlange/jeopardy/wiki/Windows)

## Play

* Edit answers/roundnumber.jrf
	* see answers/README or wiki for further instructions
* Choose round to play
* Enter names, keys and colors of players
* Select question

## Screenshots

Main:

![](http://i.imgur.com/iTd8N6o.png)

Player:

![](http://i.imgur.com/4KsajRv.png)

Colored game field:

![](http://i.imgur.com/AwaO8gd.png)

## Bugs? Feature requests? Have some Beer?

Don't hesitate to contact me!

