# painless

`painless` is a header-only C++ library that provides an easy way to use interactive parameters in
your program.

New parameters are defined with a macro call that takes an **identifier** (and name) for the parameter as well as
a **default value**:
```c++
PAINLESS_PARAMETER(my_parameter, 3.14f);
```
The variable `my_parameter` can then be used as if it was a normal `float` value.

At runtime, `painless` creates a file called `/tmp/painless/my_parameter` with the following content:
``` python
3.14
# Parameter 'my_parameter'
# Default value: '3.14'
```
At the same time, it also spawns a thread that watches for modifications of that file.
Changes to the value in the first line will immediately be reflected in the running program.

## Example program

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

## Goals

`painless` is mainly intended for early exploratory development phases and does not aim to be a
feature-complete parameter handling solution.
