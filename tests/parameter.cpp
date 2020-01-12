#include <painless/parameter.h>

#include <catch2/catch.hpp>

TEST_CASE("Reproduces default value") {
  PAINLESS_PARAMETER(x_float, 3.14f);
  CHECK(*x_float == 3.14f);

  PAINLESS_PARAMETER(x_int, 42);
  CHECK(*x_int == 42);

  PAINLESS_PARAMETER(x_bool, true);
  CHECK(*x_bool == true);

  PAINLESS_PARAMETER(x_char, 'z');
  CHECK(*x_char == 'z');
}
