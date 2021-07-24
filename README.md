# AppCUI

![Build icon](https://github.com/gdt050579/AppCUI/actions/workflows/ci.yml/badge.svg)


![AppCUI logo](https://raw.githubusercontent.com/gdt050579/AppCUI/main/docs/logo.png)

This logo is an [application](https://github.com/gdt050579/AppCUI/tree/main/Examples/Logo) made with `AppCUI`

## Building

The project will require SDL and SDL_TTF to work.

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

On Windows
```
https://www.libsdl.org/download-2.0.php -> Development Libraries (ex: SDL2-devel-2.0.14-VC.zip)
https://www.libsdl.org/projects/SDL_ttf/ -> Development Libraries (ex: SDL2_ttf-devel-2.0.15-VC.zip)
```

If the libraries are already installed in non-standard paths, use `-DCMAKE_PREFIX_PATH` to specify the path to libraries folders.

For example (from our github workflow on windows)
```
    cmake <other options> -DCMAKE_PREFIX_PATH="C:\SDL\SDL2-2.0.14;C:\SDL\SDL2_ttf-2.0.15"
```