# painless

[![Build Status](https://travis-ci.org/sharkdp/painless.svg?branch=master)](https://travis-ci.org/sharkdp/painless)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

`painless` is a header-only C++ library that provides an easy way to use interactive parameters in
your program.

New parameters are defined with a macro call that takes an **identifier** (and name) for the parameter as well as
a **default value**:
```c++
PAINLESS_PARAMETER(my_parameter, 3.14f);
```
The variable `my_parameter` can then be used as if it was a normal `float` value.

At runtime, `painless` then creates a file called `/tmp/painless/my_parameter` with the following content:
``` python
3.14
# Parameter 'my_parameter'
# Default value: '3.14'
```
In the background, it spawns a thread that watches for modifications to that file.
Changes to the value will be immediately reflected in the running program.

## Example

```c++
#include <painless/parameter.h>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  PAINLESS_PARAMETER(count, 10);
  PAINLESS_PARAMETER(message, "X");

  while (true) {
    // Print the 'message', 'count' times
    for (int i = 0; i < count; ++i) {
      std::cout << message;
    }
    std::cout << "\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
```
While this program runs, the files `/tmp/painless/message` and `/tmp/painless/count` can be
edited in order to interactively change the values of the `message` and `count` parameters.

## Goals

`painless` is mainly intended for early exploratory development phases and does not aim to be a
feature-complete parameter handling solution.
