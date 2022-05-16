# polaris-lang

A small lisp-clone language. 

## Building Polaris

**Dependencies**

 - CMake >= 3.5
 - CPPUTest - Check the `scripts` directory for an installation script, alternatively if on mac it can be installed with `brew install cpputest`

**Compiling/Installing**

```
mkdir build
cmake ../build
make -j5
sudo make install
```

Optionally `ccmake` can be used instead of `cmake` in the above to get a curses gui for configuring polaris.

**Installing stdlib**

At the time of writing this the stdlib isn't fully developed.
Enabling the cmake option `SETUP_POLARIS` and having the `HOME` environment variable set will permit the build system to copy a `.polaris/stdlib` to the `HOME` directory. Afterwards `.polaris` can be added to the include directories (below) to access files in the stdlib.

## Running Polaris

Not supplying a file will start the REPL.

**./polaris**
```
polaris> (+ 3 (/ 10 2))
8
```

**./polaris hello-world.pol**

```
./polaris hello-world.pol
Hello World!
```

**Adding include directories**

To allow easy importing polaris can be given a set of include directories to look for files
when an *import* command is executed.

```
./polaris --include dir_1:dir2:dir_3 
```

Again, if no file is present REPL will start.

```
polaris> (import "some_file.pol")
```

With the above statement, the first instance of *some_file.pol* found will be used from either the 
immediate execution directory, or one of the 3 included directories.

As with everything, the import command reads in a list so multiple things can be imported in the same statement

```
polaris> (import "some_file.pol" "some_other_file.pol")
```

The full path name of a file is recorded to ensure files are not imported multiple times.

## Docker

**Building**

`docker build -t polaris .`

**Running Container REPL**

`docker run -a stdin -a stdout -i -t polaris /polaris/docker-build/polaris`

**Credits**

- Inspiration : [Lisp interpreter in 90 lines of code](http://howtowriteaprogram.blogspot.com/2010/11/lisp-interpreter-in-90-lines-of-c.html)