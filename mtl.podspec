{
  "name": "mtl",
  "version": "0.16.0",
  "summary": "C++17 Actor Model Library",
  "description": "Multithreading Library is an open source C++17 actor model implementation featuring lightweight & fast actor implementations, pattern matching for messages, network transparent messaging, and more.",
  "homepage": "http://mtl.nil.foundation/",
  "license": "Boost Software License",
  "authors": {
    "Dominik Charousset": "dominik.charousset@haw-hamburg.de",
    "Mikhail Komarov": "nemo@nil.foundation"
  },
  "platforms": {
    "ios": "5.0",
    "osx": "10.7"
  },
  "source": {
    "git": "https://github.com/nilfoundation/mtl.git",
    "branch": "master"
  },
  "xcconfig": {
    "CLANG_CXX_LANGUAGE_STANDARD": "c++17",
    "CLANG_CXX_LIBRARY": "libc++",
    "HEADER_SEARCH_PATHS": "\"${PODS_ROOT}/libs/core/include/\" \"${PODS_ROOT}/libs/io/include\" \"${PODS_ROOT}/libs/network/include\""
  },
  "requires_arc": false,
  "prepare_command": "touch libs/core/include/nil/mtl/detail/build_config.hpp",
  "subspecs": [
    {
      "name": "core",
      "source_files": "libs/core/src/*.cpp",
      "preserve_paths": [
        "libs/core/include/nil/mtl/**/*.hpp"
      ]
    },
    {
      "name": "io",
      "dependencies": {
        "mtl/core": [

        ]
      },
      "source_files": "libs/io/src/*.cpp",
      "preserve_paths": [
        "libs/io/include/nil/mtl/**/*.hpp"
      ]
    }
  ]
}