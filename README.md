# =nil; Foundation's Actor Model Implementation Library

=nil; Actor is an open source C++ actor model implementation featuring lightweight & fast actor implementations, pattern matching for messages, network transparent messaging, and more.

## Building

=nil; Actor uses [CMake](http://www.cmake.org/) build system and follows standard procedure for building:

```
git clone --recurse-submodules https://github.com/nilfoundation/actor.git actor
cd mtl && mkdir build && cd build && cmake .. && make all
```

### CMake build options available

* ```BUILD_TESTS``` - enables building unit tests
* ```BUILD_SHARED_LIBS``` - enables building shared libraries (CMake default variable)
* ```BUILD_WITH_CCACHE``` - enables building with CCache usage.

## Dependencies

* CMake
* Boost ( >= 1.58)
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
* CCache (for caching the build)
