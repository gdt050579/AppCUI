Short (not) unix terminal and terminal introduction
1. https://en.wikipedia.org/wiki/Terminal_emulator
2. https://en.wikipedia.org/wiki/Xterm (the default terminal emulator on most systems)
3. https://en.wikipedia.org/wiki/ANSI_escape_code
4. https://en.wikipedia.org/wiki/Termcap
5. https://en.wikipedia.org/wiki/Terminfo
6. https://tldp.org/HOWTO/Text-Terminal-HOWTO-16.html
7. https://invisible-island.net/xterm/terminfo.html

The basic gist is as follows:

You have 3 parts of the application:
1. The terminal (usually a program that runs by itself and can run other programs)
2. The application that wants to output/draw something on the terminal it runs on
3. A helper library like ncurses

What usually happens:
1. You init ncurses using its functions
2. Ncurses will find out the terminal you're running from (for example xterm-256color) and check its capabilities,
    for example: can it display 256 colors or 8? how many color pairs can it store? what mouse events does it send.
3. Along with capabilities, every terminal also defines what control sequences to send so that it can perform
    those capabilities
    For example, to move the cursor to a certain line L and column C, your program has to send "\033[<L>;<C>H"
4. But you don't have to do it manually, ncurses does it for you, by reading your terminal capabilities file
    and finding all kinds of commands using their names.
    For example "clear=\E[H\E[​2J,", which means that for ncurses to do a screen clear, it can send a
    "\E[H\E[​2J" control sequence to the terminal and it will be interpreted as a clear command. The command
    names are predefined by a standard, but are optional to be implemented by terminals. If a terminal doesn't define
    a clear capability, it will not be executed when you use ncurses

What we have:
1. xterm-256color that can display 256 colors
2. xterm-1003 that can send mouse position update, as long as properly handle press and release functionality
3. They kiss and make a baby

Something like

```
xterm-256color-mouse|xterm with 256 colors and mouse,
	use=xterm-256color, use=xterm-1003,
```
