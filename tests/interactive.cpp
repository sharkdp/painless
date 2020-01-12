#include <iostream>
#include <iomanip>
#include <painless/parameter.h>

template <typename T>
void showParameter(const painless::Parameter<T>& parameter) {
  std::cout << parameter.name() << " = " << *parameter << "\n";
}

int main() {
  PAINLESS_PARAMETER(demo_float, 1.4f);
  PAINLESS_PARAMETER(demo_int, 42);
  PAINLESS_PARAMETER(demo_bool, false);
  PAINLESS_PARAMETER(demo_string, std::string("hello world"));

  int iteration = 0;
  while (true) {
    usleep(100000);

    std::cout << "================ " << std::setw(6) << iteration << " ================\n";
    showParameter(demo_float);
    showParameter(demo_int);
    showParameter(demo_bool);
    showParameter(demo_string);
    std::cout << "\n";

    ++iteration;
  }
}
