# AppCUI

![Build icon](https://github.com/gdt050579/AppCUI/actions/workflows/cmake.yml/badge.svg)


## Building for UNIX systems

On Linux and MacOS - the project will require ncurses to work. If you have it installed an it's the newest version
(6.2 as of writing) - everything should work smoothly. Otherwise, you may need to tell cmake the path to your ncurses"

This is done by setting the `-DCMAKE_PREFIX_PATH`

For example: `cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/ncurses/6.2`

Also `rm -rf` the build folder and reconfigure again to remove the cache