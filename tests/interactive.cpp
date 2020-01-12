#include <painless/parameter.h>

#include <iomanip>
#include <iostream>

template <typename T>
void showParameter(const painless::Parameter<T>& parameter) {
  std::cout << "\x1b[33m" << parameter.name() << "\x1b[0m"
            << " = "
            << "\x1b[32m" << painless::printer::to_string(*parameter)
            << "\x1b[0m\n";
}

int main() {
  PAINLESS_PARAMETER(demo_bool, false);
  PAINLESS_PARAMETER(demo_int, 42);
  PAINLESS_PARAMETER(demo_float, 1.4f);
  PAINLESS_PARAMETER(demo_string, "hello world");

  int iteration = 0;
  while (true) {
    usleep(100000);

    std::cout << "\x1b[2J\x1b[0;0H";
    std::cout << "\x1b[1mIteration\x1b[0m " << std::setw(6) << iteration
              << "\n\n";
    showParameter(demo_bool);
    showParameter(demo_int);
    showParameter(demo_float);
    showParameter(demo_string);
    std::cout << "\n";

    ++iteration;
  }
}
