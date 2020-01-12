#include <painless/parameter.h>

#include <chrono>
#include <iostream>
#include <thread>

int main() {
  PAINLESS_PARAMETER(count, 3);
  PAINLESS_PARAMETER(message, "hello");

  while (true) {
    for (int i = 0; i < *count; ++i) {
      std::cout << *message;
    }
    std::cout << "\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
