#include <gtest/gtest.h>

#include "vita/telemetry.hpp"

namespace
{
using namespace vita::telemetry;

class TelemetryTest : public ::testing::Test
{
};

TEST_F(TelemetryTest, ResourceFactoryCorrectlySetsAttributes)
{
    // I am choosing designated initializers to test the ResourceConfig struct
    ResourceConfig config{.service_name = "test-service", .service_version = "v1.2.3"};

    auto resource = CreateResource(config);
    auto attributes = resource.GetAttributes();

    // Verify service name
    EXPECT_EQ(opentelemetry::nostd::get<std::string>(attributes.at("service.name")),
              "test-service");

    // Verify service version
    EXPECT_EQ(opentelemetry::nostd::get<std::string>(attributes.at("service.version")), "v1.2.3");
}

TEST_F(TelemetryTest, RAII_CleanupDoesNotCrashOnNull)
{
    // Testing the nullptr_t constructor you implemented
    EXPECT_NO_THROW({
        TelemetryCleanUp<int> guard(nullptr);
        guard.CleanUp();
    });
}

} // namespace