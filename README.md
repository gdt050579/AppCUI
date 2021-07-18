# AppCUI

![Build icon](https://github.com/gdt050579/AppCUI/actions/workflows/ci.yml/badge.svg)


![AppCUI logo](https://raw.githubusercontent.com/gdt050579/AppCUI/main/docs/logo.png)

This logo is an [application](https://github.com/gdt050579/AppCUI/tree/main/Examples/Logo) made with `AppCUI`

## Building for UNIX systems

On Linux and MacOS - the project will require SDL and SDL_TTF to work.

On OSX
```
brew install sdl2
brew install sdl2_ttf
```

On Linux
```
sudo apt-get install libsdl2-dev -y
sudo apt-get install libsdl2-ttf-dev -y
```

If the libraries are already installed in non-standard paths, use `-DCMAKE_PREFIX_PATH` to specify the path to libraries folders.
