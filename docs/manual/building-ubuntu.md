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
