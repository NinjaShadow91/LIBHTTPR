# LIBHTTPR - C++ HTTP Routing Library

This is a header-only C++ library for routing, inspired by the Express.js framework. It provides a simple and efficient way to handle HTTP requests and responses in your C++ applications.

## Features

- Header-only: No need to build or link against a library. Just include the header files in your project.
- Express-like API: If you're familiar with Express.js, you'll find this library easy to use.
- Middleware support: Easily add middleware functions to handle requests and responses.
- Static file serving: Serve static files like HTML, CSS, and images.
- Uses modern C++17 features for better performance and easier use.
- Built on top of Boost libraries for robustness and reliability.

## Dependencies

This library depends on several Boost libraries:

- Boost.System
- Boost.Thread
- Boost.Filesystem
- Boost.Regex

## Usage

To use this library, simply include the necessary header files in your C++ source file:

```cpp
#include "PATH_TO_LIB/include/libhttpr.hpp"
```

Then, compile your source file with g++. You need to link against the necessary Boost libraries and enable C++17:

```bash
g++ your_source_file.cpp -lboost_system -lboost_thread -lpthread -lboost_filesystem -lboost_regex --std=c++17
```

Replace your_source_file.cpp with the name of your source file.

## Example

You can find an example usage of this library in the example.cpp file.

## License

This library is released under the MIT License. See the LICENSE file for more details.
