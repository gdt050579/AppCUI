# AppCUI

![Build icon](https://github.com/gdt050579/AppCUI/actions/workflows/ci.yml/badge.svg)


![AppCUI logo](https://raw.githubusercontent.com/gdt050579/AppCUI/main/docs/logo.png)

This logo is an [application](https://github.com/gdt050579/AppCUI/tree/main/Examples/Logo) made with `AppCUI`

## Building

Optionally, you can choose which frontend you want to have on your system:

On Unix and OSX:
- `SDL2` - full functionality, runs in a separate window
- `ncurses` - limited functionality, runs in any terminal

On Windows:
- `SDL2` - full functionality, runs in a separate window
- `Windows Console` - full functionality, runs in the console

### Installing SDL

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

If everything is installed properly, cmake will greet you with `[cmake] Have SDL2`


### Installing Ncurses

On Unix and OSX - the project can also use ncurses. If you have it installed an it's the newest version (6.2 as of writing) - everything should work smoothly. Otherwise, you may need to tell cmake the path to your ncurses

This is done by setting the `-DCMAKE_PREFIX_PATH`

For example: `cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2`

If everything is installed properly, cmake will greet you with `[cmake] Have Curses`


## Documentation 

The project uses Sphinx as the main documentation engine. Sphinx sources can be located under `docs` folder. Local building is done with `make html`, after which the html page can be found in the `build` folder.

On every commit to `main`, a compiled version of the Sphinx documentation is published to `gh-pages` and then to `https://gdt050579.github.io/AppCUI/`