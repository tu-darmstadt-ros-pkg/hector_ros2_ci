#include <gtest/gtest.h>

#include <string>
#include <vector>

// Gives the clang-tidy step of the CI a translation unit to check
TEST(Smoke, builds_and_runs)
{
  const std::vector<std::string> words{"hector", "ci"};
  EXPECT_EQ(words.size(), 2u);
}
