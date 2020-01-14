#include <painless/parameter.h>

#include <catch2/catch.hpp>

TEST_CASE("Reproduces default value") {
  PAINLESS_PARAMETER(default_value_float, 3.14f);
  CHECK(default_value_float == 3.14f);

  PAINLESS_PARAMETER(default_value_int, 42);
  CHECK(default_value_int == 42);

  PAINLESS_PARAMETER(default_value_bool, true);
  CHECK(default_value_bool == true);

  PAINLESS_PARAMETER(default_value_char, 'z');
  CHECK(default_value_char == 'z');

  PAINLESS_PARAMETER(default_value_string, "hello world");
  CHECK(*default_value_string == "hello world");

  PAINLESS_PARAMETER(default_value_int8_t, int8_t{4});
  CHECK(default_value_int8_t == 4);
}

TEST_CASE("can be used with comparison operators") {
  PAINLESS_PARAMETER(value1, 1.5f);

  CHECK(value1 == 1.5f);
  CHECK(value1 != 0.0f);
  CHECK(value1 > 1.0f);
  CHECK(value1 >= 1.0f);
  CHECK(value1 < 2.0f);
  CHECK(value1 <= 2.0f);
}

TEST_CASE("can be used in expressions") {
  PAINLESS_PARAMETER(value2, 7);

  CHECK(value2 + 3 == 10);
  CHECK(3 + value2 == 10);
  CHECK(value2 * 3 == 21);
  CHECK(3 * value2 == 21);
}

template <typename T>
void writeToParameterFile(const painless::Parameter<T>& parameter,
                          const std::string& content) {
  {
    std::string path = painless::get_base_path();
    path += parameter.name();
    std::ofstream f(path);
    f << content << "\n";
  }
}

template <typename T, typename T2>
void waitForValue(const painless::Parameter<T>& parameter, T2 expected_value) {
  const size_t max_waiting_iterations = 100;  // each iteration is 100 us
  for (size_t i = 0; i < max_waiting_iterations; ++i) {
    if (*parameter == expected_value) {
      SUCCEED();
      return;
    }
    usleep(100);
  }

  FAIL("Exceeded maximum waiting time for parameter '"
       << parameter.name() << "'. expected_value = " << expected_value
       << "    parameter = " << parameter);
}

TEST_CASE("Reads updated parameter from file") {
  PAINLESS_PARAMETER(update_float, 0.f);
  writeToParameterFile(update_float, "1.23f");
  waitForValue(update_float, 1.23f);

  PAINLESS_PARAMETER(update_int, 0);
  writeToParameterFile(update_int, "42");
  waitForValue(update_int, 42);

  PAINLESS_PARAMETER(update_bool, false);
  writeToParameterFile(update_bool, "true");
  waitForValue(update_bool, true);

  PAINLESS_PARAMETER(update_char, 'a');
  writeToParameterFile(update_char, "b");
  waitForValue(update_char, 'b');

  PAINLESS_PARAMETER(update_string, "foo bar");
  writeToParameterFile(update_string, "foo bar baz");
  waitForValue(update_string, "foo bar baz");

  PAINLESS_PARAMETER(update_int8_t, 1);
  writeToParameterFile(update_int8_t, "2");
  waitForValue(update_int8_t, 2);
}

TEST_CASE("Can handle multiple value updates") {
  PAINLESS_PARAMETER(multiple_value_int, 0);

  for (int i = 1; i <= 10; ++i) {
    writeToParameterFile(multiple_value_int, std::to_string(i));
    waitForValue(multiple_value_int, i);
  }

  for (int i = 11; i <= 20; ++i) {
    writeToParameterFile(multiple_value_int, std::to_string(i));
  }
  waitForValue(multiple_value_int, 20);
}

TEST_CASE("Yields default value on empty input") {
  PAINLESS_PARAMETER(parsing_float, 1.1f);
  writeToParameterFile(parsing_float, "2.2");
  waitForValue(parsing_float, 2.2f);

  writeToParameterFile(parsing_float, "");
  waitForValue(parsing_float, 1.1f);
}

TEST_CASE("Yields default value on parse error") {
  PAINLESS_PARAMETER(parsing_float, 1.1f);
  writeToParameterFile(parsing_float, "2.2");
  waitForValue(parsing_float, 2.2f);

  writeToParameterFile(parsing_float, "dummy");
  waitForValue(parsing_float, 1.1f);
}

TEST_CASE("Can re-use parameter name") {
  {
    PAINLESS_PARAMETER(reuse_float, 1.1f);
    CHECK(reuse_float == 1.1f);
    writeToParameterFile(reuse_float, "2.2f");
    waitForValue(reuse_float, 2.2f);
  }
  {
    PAINLESS_PARAMETER(reuse_float, 3.3f);
    CHECK(reuse_float == 2.2f);  // The 'reuse_float' parameter is persistent,
                                 // no new initialization is performed
    writeToParameterFile(reuse_float, "4.4f");
    waitForValue(reuse_float, 4.4f);
  }
}

int get_shared_parameter_value() {
  PAINLESS_PARAMETER(shared_parameter, 3);
  return shared_parameter;
}

TEST_CASE("Can share parameters across scopes") {
  PAINLESS_PARAMETER(shared_parameter, 1);

  CHECK(shared_parameter == 1);

  writeToParameterFile(shared_parameter, "2");
  waitForValue(shared_parameter, 2);

  // Make sure that the initial file is not written again:
  CHECK(get_shared_parameter_value() == 2);
}
