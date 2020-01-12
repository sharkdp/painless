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

void writeToFile(const char* path, const std::string& content) {
  {
    std::ofstream f(path);
    f << content << "\n";
  }
  usleep(100000);
}

TEST_CASE("Reads updated parameter from file") {
  PAINLESS_PARAMETER(update_float, 3.14f);
  CHECK(*update_float == 3.14f);
  writeToFile("/tmp/painless/update_float", "1.23f");
  CHECK(*update_float == 1.23f);
}

TEST_CASE("Can re-use parameter name") {
  {
    PAINLESS_PARAMETER(reuse_float, 1.1f);
    CHECK(*reuse_float == 1.1f);
    writeToFile("/tmp/painless/reuse_float", "2.2f");
    CHECK(*reuse_float == 2.2f);
  }
  {
    PAINLESS_PARAMETER(reuse_float, 3.3f);
    CHECK(*reuse_float == 3.3f);
    writeToFile("/tmp/painless/reuse_float", "4.4f");
    CHECK(*reuse_float == 4.4f);
  }
}
