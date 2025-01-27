#include <string>

#include "xi/xi.hpp"

auto main() -> int
{
  auto const exported = exported_class {};

  return std::string("xi") == exported.name() ? 0 : 1;
}
