## Cpp Command Mark

A simple cli tool to memorize command along with where it should be ran.

## build

the program uses cmake to build itself and uses vcpkg as its library manager. Makesure you've setted them properly. Also, you need install ncurses and fmt libarary in vcpkg

than, in terminal, clone the project into a directory:

```
    cd CppCommandMark
    cmake --build ./cmake-build-debug --target CppCommandMark
```

if everything goes well, you will find the executable in cmake-build-debug

## Usage

usage:
```
    ./CppCommandMark <option> [args] 
```
Options:
```
	--help(-h): show this help
 
	--check(-c): choose command and use ( default )
 
	--delete(-d): choose command and delete
 
	--mark(-x): mark command
 
		--directory: manually choose where the command will run
```
extra options:
```
	--verbose(-v): show verbose output
 
	--file(-f): manually choose where the information file is
```
## Details

the program uses a few libraries that isn't STL:

1. [fuzzy match](https://github.com/philj56/fuzzy-match) \(this project has contained its source for convenience\)
2. curses \(also known as ncurses\)
3. fmt

Besides, the program uses ioctl\(\) to write STDIN_FILENO (`ioctl(STDIN_FILENO, TIOCSTI, &c)`) to leave text in terminal for convenience. Note that on some modern Linux core this is disabled by default. I'm trying to change these code.

In case you really want to use this feature right now, using this command to enable it.

```
    sudo sysctl -w dev.tty.legacy_tiocsti=1
```

## plan of version 2

- highlight of matched characters
- more acceptable way to leave text in terminal

## plan of future

- more kind of tasks