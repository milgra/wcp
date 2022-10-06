# wcp - Wayland Control Panel

Script-driven control panel/system menu for Wayland compositors.

Wayland Control Panel was made for SwayOS (https://swayos.github.io)

![alt text](screenshot.png)

## Features ##

- content and style fully configurable via html, css and shell scripts
- super lightweight ( no gtk&qt )

## Installation

### Compiling from source

Install dependencies:

- wayland
- freetype2 \*
- wayland-protocols \*
- meson \*
- png \*

\* _compile-time dependecy_

Run these commands:

```
git clone git@github.com:milgra/wcp.git
cd wcp
meson build --buildtype=release
ninja -C build
sudo ninja -C build install
```

### From packages

[![Packaging status](https://repology.org/badge/tiny-repos/wcp.svg)](https://repology.org/project/wcp/versions)

## Usage

Launch wcp in a terminal, enter 0 to hide, 1 to show, 2 to toggle, 3 to quit wcp and press return.

```
wcp
```

### Usage with sway wm

Launch wcp in the config connected to a named pipe, but remove the named pipe first to avoid mkfifo errors.

```
exec rm -f /tmp/wcp && mkfifo /tmp/wcp && tail -f /tmp/wcp | wcp
```

Set up your sway config or menu bar config to toggle wcp

For example, to toggle with META+P
```
bindsym $mod+p exec "echo 2 > /tmp/wcp"
```

## Contribution/Development ##

Feel free to push fixes/improvements.

Please follow these guidelines :

- use clang format before commiting/after file save
- use zen_core functions and containers and memory handling
- make sure that the app is leak free. if you run the dev build it automagically checks for leaks on exit on two levels (zc_memory and clang address sanitizer ) and prints leaks

Creating a debug build :

```
CC=clang meson build --buildtype=debug -Db_sanitize=address -Db_lundef=false
ninja -C build
```

## Donate ##

paypal : [https://paypal.me/milgra](https://paypal.me/milgra)  
patreon : [https://www.patreon.com/milgra](https://www.patreon.com/milgra)  
bitcoin : 37cSZoyQckihNvy939AgwBNCiutUVN82du  

## License ##

MIT, see [LICENSE](/LICENSE).

## TODO/BUGS ##

fix test and release workflows  
