# painless

`painless` is a header-only library that provides an easy way to use interactive parameters in your program.

New parameters are defined with a macro call that takes an identifier (and name) for the parameter as well as a default value:
```c++
PAINLESS_PARAMETER(my_parameter, 3.14f);
```
After this, `my_parameter` can be used as if it was a normal `float` value. When the corresponding program runs, `painless`
creates a file called `/tmp/painless/my_parameter` with the following content:
```
3.14
# Parameter 'my_parameter'
# Default value: '3.14'
```
It also spawns a thread that watches for changes to that file. Whenever you change the value in the first line, the
corresponding value will immediately change in the running program.

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
