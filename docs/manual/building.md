# Building =nil; Actor

## Building =nil; Actor on Arch

Installing required packages:

```
sudo ./install-dependencies.sh
```

To compile =nil; Actor use:

```
./configure.py --mode=release
ninja -C build
```

## Building =nil; Actor on CentOS

### Building =nil; Actor on CentOS 7

Installing required packages:

```
sudo ./install-dependencies.sh
./cooking.sh -r dev -i c-ares -i fmt -t Release
```

To compile =nil; Actor explicitly using gcc 5, use:

```
CXX=/opt/scylladb/bin/g++ ./cooking.sh -i c-ares -i fmt -t Release
ninja-build -C build
```

## Building =nil; Actor on Fedora

### Building =nil; Actor on Fedora 21 and later

Installing required packages:

```
sudo ./install-dependencies.sh
```

You then need to run the following to create the build directory:

```
./configure.py --mode=release
```

Note it is enough to run this once, and you don't need to repeat it before
every build.

Then finally:

```
ninja-build -C build/release
```

In case there are compilation issues, especially like ```g++: internal compiler error: Killed (program cc1plus)``` try
giving more memory to gcc, either by limiting the amount of threads ( -j1 ) and/or allowing at least 4g ram to your
machine

## Building =nil; Actor on Ubuntu

### Building =nil; Actor on Ubuntu 14.04/15.10/16.04

Installing required packages:

```
sudo ./install-dependencies.sh
```

To compile =nil; Actor explicitly using gcc 5, use:

```
CXX=g++-5 ./cooking.sh -i c-ares -i fmt -t Release
ninja -C build
```

## Building with a DPDK network backend

1. Setup host to compile DPDK:
    - Ubuntu
      `sudo apt-get install -y build-essential linux-image-extra-$(uname -r$)`
2. Configure the project with DPDK enabled: `./configure.py --mode=release --enable-dpdk`
3. Run `ninja-build build/release`.

To run with the DPDK backend for a native stack give the actor application `--dpdk-pmd 1` parameter.

You can also configure DPDK as an [external package](README-DPDK.md).

## Building =nil; Actor in Docker container

To build a Docker image:

```
docker build -t actor-dev docker/dev
```

Create an shell function for building insider the container (bash syntax given):

```
$ seabuild() { docker run -v $HOME/actor/:/actor -u $(id -u):$(id -g) -w /actor -t actor-dev "$@"; }
```

(it is recommended to put this inside your .bashrc or similar)

To build inside a container:

```
$ seabuild ./configure.py
$ seabuild ninja -C build/release
```