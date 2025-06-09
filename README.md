# led (lauchi's editor)
A rudimentary text editor made with [raylib](https://github.com/raysan5/raylib)
![](https://raw.githubusercontent.com/lauchimoon/led/refs/heads/main/assets/ss.png)

## Keybinds
- Ctrl + Q: Exit editor
- Ctrl + D: Delete current line
- Ctrl + S: Save buffer
- Ctrl + Z: Undo (limited for now)
- Ctrl + K: Increment font size
- Ctrl + J: Decrement font size
- Ctrl + T + <1, 2>: Change theme (1 is dark, 2 is white)

## Usage
```
$ git clone https://github.com/lauchimoon/led.git
$ cd lame
$ make
$ ./led <file>
```
If `<file>` doesn't exist, `led` will create it.
