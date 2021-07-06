# AppCUI

![Build icon](https://github.com/gdt050579/AppCUI/actions/workflows/ci.yml/badge.svg)


![AppCUI logo](https://raw.githubusercontent.com/gdt050579/AppCUI/main/docs/logo.png)

This logo is an [application](https://github.com/gdt050579/AppCUI/tree/main/Examples/Logo) made with `AppCUI`

## Building for UNIX systems

On Linux and MacOS - the project will require ncurses to work. If you have it installed an it's the newest version
(6.2 as of writing) - everything should work smoothly. Otherwise, you may need to tell cmake the path to your ncurses

This is done by setting the `-DCMAKE_PREFIX_PATH`

For example: `cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2`

Also `rm -rf` the build folder and reconfigure again to remove the cache

## Setting up a proper terminal

If you wish to just have mouse button press functionality without drag, you can stick with `xterm-256color`, without
chaning anything

Details can be found in `docs/terminal/TERMINAL.md`

We would need the terminal capabilities for `256 colors` and the terminal capabilities for proper mouse events like
press, released and also position reporting on every more (for drag functionality).

The color part can be implemented by `xterm-256color` which comes by default in `Debian` and most terminals on unix systems (or can be setup if it's not default).

For mouse events we can use `xterm-1003` capabilities, the `kmous`, `XM` and `xm` ones, to be precise

Thus, we have crafted the following terminal capabilities file - `docs/terminal/xterm-256color-mouse`

### Unix 

1. Run the terminfo compiler `tic -x docs/terminal/xterm-256color-mouse`
2. Set the current terminal `export TERM=xterm-256color-mouse`
    * For persistence, the following command can be added to your shell configuration
3. Run the program!

### MacOS

The same steps as on linux, but ensure you're using the proper `tic`. After installing `brew install ncurses`, the
`tic` or other terminal capabilities related programs should be run from the `bin` folder of ncurses so it can find the
capability files when running the program. For example: `miniconda` also ships the `tic` binary, which compiles the 
capabilities provided, but `ncurses` doesn't find them.
