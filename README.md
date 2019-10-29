# MTL: C++ Actor Framework

MTL is an open source C++ actor model implementation featuring lightweight & fast actor implementations, pattern matching for messages, network transparent messaging, and more. 

## Srsly? Why not to use CAF?

Changes consist of several points:

* Replacement of pseudo-reflection mechanism to versioning-resistant serialization.
* Actor network communication protocol transport level serialization parametrisation.
* Reduction of self-maintained code with extensive Boost libraries usage.

## Get the Sources

* git clone https://github.com/nilfoundation/mtl.git
* cd mtl

## Build MTL from Source

The easiest way to build MTL is to use the `configure` script. Other available
options are using [CMake](http://www.cmake.org/) directly.

### Using CMake

MTL also can be included as CMake submodule or added as dependency to other
CMake-based projects using the file `cmake/FindMTL.cmake`.

## Dependencies

* CMake
* Pthread (until C++11 compilers support the new `thread_local` keyword)

## Supported Compilers

* GCC >= 4.8.3
* Clang >= 3.2
* MSVC >= 2015, update 3

## Supported Operating Systems

* Linux
* Mac OS X
* FreeBSD 10
* Windows >= 7 (currently static builds only)

## Optional Dependencies

* Doxygen (for the `doxygen` target)
* LaTeX (for the `manual` target)
* Pandoc and Python with pandocfilters (for the `rst` target)
