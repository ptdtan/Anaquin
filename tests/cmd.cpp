#include <catch.hpp>
#include "test.hpp"

using namespace Anaquin;

TEST_CASE("Anaquin_RnaAlign_HelpShort")
{
    REQUIRE(Test::test("RnaAlign -h").status == 0);
}

TEST_CASE("Anaquin_RnaAlign_HelpLong")
{
    REQUIRE(Test::test("RnaAlign --help").status == 0);
}