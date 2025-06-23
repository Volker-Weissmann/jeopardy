# Jeopardy

* Original Author:  Christian Lange (Christian_Lange@hotmail.com)
* Date:             06. February 2014
* Version:          0.9.6 Stable
* Github:           https://github.com/Volker-Weissmann/jeopardy
* Homepage:         http://ganz-sicher.net/chlange
* License:          New BSD License (3-clause BSD license)

# Description

* Implementation of well known Jeopardy! quiz show in C++ with Qt.

# Features

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

# How to build and run

Beware that it will not work if you cd into different directory.

## ArchLinux

### Using Qt6

* `pacman -S meson qt6-base qt6-multimedia`
* `meson setup bd -Dqt_version=6`
* `ninja -C bd`
* `./bd/jeopardy`

### Using Qt5

* `pacman -S meson qt5-base qt5-multimedia`
* `meson setup bd -Dqt_version=5`
* `ninja -C bd`
* `./bd/jeopardy`

## NixOS

### Using Qt6

* `nix-shell -p pkgs.qt6.full meson ninja`
* `meson setup bd -Dqt_version=6`
* `ninja -C bd`
* `./bd/jeopardy`

### Using Qt5

Note: [Video Playback does not work when using Qt5 on NixOS. I'm sorry.](https://forum.qt.io/topic/162479/qmediaplayer-does-not-work-on-nixos/3)

Note: If you install `qt5Full` and `pkgs.qt6.full`, then building with `-Dqt_version=5` will unfortunately not work.

* `nix-shell -p qt5Full meson ninja`
* `meson setup bd -Dqt_version=5`
* `ninja -C bd`
* `./bd/jeopardy`

## Windows
* I do not now if it works on Windows, all I could find was [this old link](https://github.com/chlange/jeopardy/wiki/Windows)

# Play

* Edit answers/roundnumber.jrf
	* see answers/README or wiki for further instructions
* Choose round to play
* Enter names, keys and colors of players
* Select question

# Screenshots

Main:

![](http://i.imgur.com/iTd8N6o.png)

Player:

![](http://i.imgur.com/4KsajRv.png)

Colored game field:

![](http://i.imgur.com/AwaO8gd.png)

# Bugs? Feature requests? Have some Beer?

Don't hesitate to contact me!
