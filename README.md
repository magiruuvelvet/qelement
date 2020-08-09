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

## Limitations

 - Native extensions made for the Electron version can't be used in QElement and never will be.
   When using QElement only the JS/WASM OLM implementation for E2E is available.

## TODO

 - Probably show a counter in the notification tray icon, currently just a red circle.

## Building

```sh
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## How to use?

QElement requires the built web app found at the [element-web](https://github.com/vector-im/element-web/releases) repository;
the extracted Electron `webapp.asar` works too. Make sure a `config.json` file is available in the web app root.
By default QElement will look for the web app in `/opt/Element/resources/webapp`, but the location can be customized
in the config file found at `~/.local/share/QElement/<profile>/preferences.ini`.

**Default Configuration**

```ini
[app]
sysTrayIconEnabled=true

[element]
webroot=/opt/Element/resources/webapp
```

## Installing

For binary releases I only support flatpak on Linux. My applications are hosted on my own flatpak repository.

```
flatpak remote-add --if-not-exists magiruuvelvet https://flatpak.magiruuvelvet.gdn/magiruuvelvet.flatpakrepo
flatpak install gdn.magiruuvelvet.QElement
```
