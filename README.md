# QElement

QElement is an alternative desktop app implementation for
[Element (formerly known as Vector and Riot)](https://github.com/vector-im/riot-web)
which uses the Qt Web Engine instead of Electron.

## Supported Features and differences to the Electron version

 - Desktop Notifications just like the Electron version, including sound.
 - A functional system tray icon which just works™ (Electron has a very very horrible system tray implementation).
 - Respects your operating system file pickers (KDE, GNOME, whatever) while Electron has Gtk file pickers hardcoded.

## Issues

 - Clicking on the desktop notification may not work depending on your desktop environment.
   Investigation is ongoing, could be Qt bug or not.

## TODO

 - Indicate network errors in the system tray icon just like the Electron app.
 - Probably show a counter in the notification tray icon, currently just a red circle.

## Building

```sh
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
