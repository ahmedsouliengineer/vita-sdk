#include <cstdlib>
#include <gtest/gtest.h>

#include "vita/config.hpp"

// I am choosing to use the explicit namespace to ensure the linter
// can map GetEnv and EnvProfile to the vita-sdk library.
using namespace vita::config;

namespace vita::test
{

class ConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        unsetenv("VITA_ENV");
        unsetenv("VITA_DEPLOYMENT_MODE");
    }
};

TEST_F(ConfigTest, FetchesString)
{
    setenv("VITA_DEPLOYMENT_MODE", "prod", 1);
    EXPECT_EQ(GetDeploymentEnv(), "prod");
}

TEST_F(ConfigTest, MapsProfile)
{
    setenv("VITA_ENV", "edge", 1);
    EXPECT_EQ(GetProfile(), EnvProfile::Edge);
}

} // namespace vita::test