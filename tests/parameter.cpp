#include <painless/parameter.h>

#include <catch2/catch.hpp>

TEST_CASE("Reproduces default value") {
  PAINLESS_PARAMETER(default_value_float, 3.14f);
  CHECK(*default_value_float == 3.14f);

  PAINLESS_PARAMETER(default_value_int, 42);
  CHECK(*default_value_int == 42);

  PAINLESS_PARAMETER(default_value_bool, true);
  CHECK(*default_value_bool == true);

  PAINLESS_PARAMETER(default_value_char, 'z');
  CHECK(*default_value_char == 'z');
}

template <typename T>
void writeToParameterFile(const painless::Parameter<T>& parameter,
                          const std::string& content) {
  {
    std::string path = painless::BASE_PATH;
    path += parameter.name();
    std::ofstream f(path);
    f << content << "\n";
  }
}

template <typename T>
void waitForValue(const painless::Parameter<T>& parameter, T expected_value) {
  const size_t max_waiting_iterations = 100;  // each iteration is 100 us
  for (int i = 0; i < max_waiting_iterations; ++i) {
    if (*parameter == expected_value) {
      SUCCEED();
      return;
    }
    usleep(100);
  }

  FAIL("Exceeded maximum waiting time for parameter '"
       << parameter.name() << "'. expected_value = " << expected_value
       << "    *parameter = " << *parameter);
}

TEST_CASE("Reads updated parameter from file") {
  PAINLESS_PARAMETER(update_float, 3.14f);
  writeToParameterFile(update_float, "1.23f");
  waitForValue(update_float, 1.23f);
}

TEST_CASE("Can re-use parameter name") {
  {
    PAINLESS_PARAMETER(reuse_float, 1.1f);
    CHECK(*reuse_float == 1.1f);
    writeToParameterFile(reuse_float, "2.2f");
    waitForValue(reuse_float, 2.2f);
  }
  {
    PAINLESS_PARAMETER(reuse_float, 3.3f);
    CHECK(*reuse_float == 3.3f);
    writeToParameterFile(reuse_float, "4.4f");
    waitForValue(reuse_float, 4.4f);
  }
}
