Short (not) unix terminal and terminal introduction

Reference list (most of them are required or will be helpful while reading)
1. https://en.wikipedia.org/wiki/Terminal_emulator
2. https://en.wikipedia.org/wiki/Xterm (the default terminal emulator on most systems)
3. https://iterm2.com/ (a very good terminal emulator for OSX)
4. https://en.wikipedia.org/wiki/ANSI_escape_code
5. https://en.wikipedia.org/wiki/Termcap
6. https://en.wikipedia.org/wiki/Terminfo
7. https://tldp.org/HOWTO/Text-Terminal-HOWTO-16.html
8. https://invisible-island.net/xterm/terminfo.html
9. https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
10. http://rtfm.etla.org/xterm/ctlseq.html

## Preface
This documenent is a part of ongoing effort to learn about terminals and capabilities to
be able render appcui in a very-crossplatform way.

Contents of this file are not final and in no way claim to be rigurous or even right. If 
any errors are found or more information can be added, feel free to create a pull request

## Introduction

### Terminal
It is usually an app that is able to receive commands and render the visual output on the screen.
It can be used interchangeably with `terminal emulator`. Some examples of terminal emulators are
`xterm`, `konsole`, `iterm2 (osx)`, `powershell`, `terminal.app (osx)`.


Terminals will interface with the underlying program through their `stdin` and `stdout` and
will pass and receive information that will be displayed on the screen. A very common protocol 
of communication between a terminal and a program is [ANSI codes](https://en.wikipedia.org/wiki/ANSI_escape_code).

Since the running program also has to use its stdout for data outputing, the `ANSI` escape sequences are
a way to fit metadata inside a data-transmission channel. For example, one way to send a string to the output, but
also to tell the terminal to display it in red would be to write `<red>this is red<end>but this is not`. In this example
the supposed terminal will see that a `<red>` command is passed and will display red letters until and `<end>` is found, 
after that it will display letters with their default color.

Terminal commands can be as simple as setting the color, but as complex as moving the cursor, displaying images or
anything else by the matter that can be reasonably fit in an output sequence and can be interpreted by a terminal.

A real example of outputing differently formatted text can be found in this same folder `term_test.cpp`, or the
main part is also displayed here.

```cpp
    std::cout << "\033[1mthis is going to be bold, " 
                << "\033[0m" << std::endl // Reset formatting 
                << "\033[3mbut this will be italic, "
                << "\033[0m" << std::endl
                << "\033[9mthis text should be strikethrough"
                << "\033[0m" << std::endl
                << "\033[31m\033[47mthis text should have red foreground and white background"
                << "\033[0m" << std::endl
                << "this text will not be formatted";
```

Sending `\033[1m` to `stdout` will alert the terminal that bold text will follow. Sending a `\033[0m` will reset
the formatting, and so on.

In the same manner, the terminal will send events to the application (to `stdin`) and it has to be albe to parse them,
in a meaningful way (see `ncurses` later in the documentation).

Each terminal emulator supports different types of commands and different formats of outputing, they
call those commands `capabilities`. [termcap](https://en.wikipedia.org/wiki/Termcap), [terminfo](https://en.wikipedia.org/wiki/Terminfo). Some of the terminals also support other terminal comands, like `iterm2` supports `xterm` commands.
`terminal.app` also supports `xterm` in a way, but most of them are not fully compatible.


For example, a very good and popular terminal emulator `xterm` has the capabilities to display 256 colors, capture
mouse events and send them. `xterm` will have special commands the application has to send to perform these actions,
like coloring a cell, moving the cursor, etc. It will also have special commands the application will receive that
signify events like typing a character, a mouse position change, etc.

Why are there multiple capabilities files for the same terminal? Why is there `xterm-256color`, `xterm-1003`,
`xterm-new` and others? My guess would be that as the terminal was evolving and new features were introduced -
different capabilities were added and the old capabilities files remained for compatibility reasons. A new 
feature shouldn't break old code.

### Shell
Usually a program that can be run and can do complex operations such as creating processes, piping
stuff, also usually contains a scripting language. When running a terminal in modern systems, 
it usually starts a shell, for a friendly commandline interface. Common shells are `bash`, `ssh`, `zsh`,
`fish`, `powershell` (which might also be a terminal-emulator, but windows is wierd, don't mind it)

The general structure is going to be like that (the examples uses `bash` and `xterm` but the idea is the same)

```
  read()       input <-               input (forwarded back to xterm)       write("\033[1m")
|---------| ------------- |------| ------------------------------------- |--------------------|
|  XTERM  |  -> starts    | bash |  -> user starts ./my_drawing_app      |   my_drawing_app   |
|---------| ------------- |------| ------------------------------------- |--------------------|
  write()     output ->             output (forwarded to my_drawing_app)       read() 
```

### NCurses (and curses)

`ncurses` and `curses` are libraries that can interface with terminals without the user needing to
know which escape sequences to send or even caring about which terminal emulator runs their app.

The ncurses library does the reading, writing and parsing of the information according the `capabilities`
of the terminal the app runs in. But how does ncurses know that the current terminal can display 8 colors?
Or maybe it displays 256 colors? Does the terminal capture mouse events or does it not? If each terminal
has different capabilities and different ANSI escape sequences, then how does it know which sequences to send
and how to parse input sequences?

ncurses uses [`terminfo`](https://en.wikipedia.org/wiki/Terminfo) files that describe the terminal capabilities like `colors` or `mouse events`.
It will find out which terminal it currently runs in and what capabilities it has. It might look
at the `TERM` environment variable and they look for the capabilities file in predefined locations
on the system.

Each terminfo file will list the capabilities in a very meaningful way through `key=value` sequences.

For example, if I run `infocmp -x xterm-256color`, which will give me the capabilities description
of the `xterm` terminal (that is happily portable with `iterm2` which I use on my OSX). 
As the name suggests, this terminal will be able to display 256 colors and
will contain values such as `setab=\E[%?%p1%{8}%<%t4%p1%d%e%p1%{16}%<%t10%p1%{8}%-%d%e48;5;%p1%d%;m`.

This will say to `ncurses`: *hey, know that `setab` capability that is used for setting the cell background color?,
well, if you want me to do it, please send me this long string with the colors you have to place in specific 
locations of the string and then I will parse it and display the colors*.

Basically, ncurses doesn't need to know all the formats and how to parse strings as well as how to format strings to send back.
It will know capabilities as `setab`, `setaf`, `kmous`, `XM` and will try to find them in the `terminfo` file supplied
by the current terminal it runs in. After getting the formats, it will be much easier to communicate with any terminal,
but just plugging and unplugging some values from the strings. This is actually done using `term.h` functions like
`tparam` or `putp`


## Application needs

Depeinding on your application needs, you might want to display colors or capture mouse. 
AppCUI has a lot of needs that are not implemented by all terminals. `xterm` does most of them, but
the default `xterm-256color` lacks very explicit mouse events. While it has clicks, it doesn't report explicit
mouse movements for an uneventful cursor movement (without any clicks) [stackoverflow-issue](https://stackoverflow.com/questions/29020638/which-term-to-use-to-have-both-256-colors-and-mouse-move-events-in-python-curse).

Luckily for us `xterm-256color` is just capabilities file that we can use. It has evolved a lot since then 
and other capabilities have been crafter and embedded in the terminal, like `XM` and `xm` for reporting
the mouse to the application in a very explicit way. These capabilities can be found in terminfo files
such as `xterm-1003`. 

We can create new `terminfo` files, compile them and then instruct our terminal to use them as the
go-to files when `ncurses` ask for what capabilities can it use. Usually, we can give it a new name,
such as `xterm-256color-mouse`, then compile it using `tic` (with `-x` that says we are compiling
extended capabilities), then put this name in out `TERM` variable.

The next time ncurses starts, it will see that we are using `xterm-256color-mouse`, go ahead and read
it and start reporting events and receiving commands.


What we have:
1. xterm-256color that can display 256 colors
2. xterm-1003 that can send mouse position update, as long as properly handle press and release functionality
3. They kiss and make a baby

Something like

```
xterm-256color-mouse|xterm with 256 colors and mouse,
	use=xterm-256color,
	kmous=\E[M, XM=\E[?1003%?%p1%{1}%=%th%el%;,
```
