#include <chrono>
#include <gtest/gtest.h>
#include <httplib.h>
#include <thread>

#include "vita/health.hpp"

namespace vita::test
{

using namespace vita::health;

class HealthMonitorTest : public ::testing::Test
{
protected:
    static constexpr std::chrono::milliseconds k_server_start_up_delay{100};
    static constexpr int k_test_port = 9090;

    void SetUp() override
    {
        // I am choosing to ensure a clean slate before each test.
        // Since we don't have a 'Reset' method in the header yet,
        // I am setting common components to a known state.
        auto& monitor = HealthMonitor::Instance();
        monitor.SetComponentStatus("database", Status::kHealthy);
    }
};

// I am choosing to verify that readiness requires ALL components to be healthy.
TEST_F(HealthMonitorTest, ReadinessLogic)
{
    auto& monitor = HealthMonitor::Instance();

    monitor.SetComponentStatus("network", Status::kHealthy);
    EXPECT_TRUE(monitor.IsReady());

    // I am choosing to transition one component to unhealthy.
    monitor.SetComponentStatus("database", Status::kDegraded);
    EXPECT_FALSE(monitor.IsReady());
}

// I am choosing to verify the Singleton instance is consistent.
TEST_F(HealthMonitorTest, SingletonConsistency)
{
    auto& instance1 = HealthMonitor::Instance();
    auto& instance2 = HealthMonitor::Instance();

    EXPECT_EQ(&instance1, &instance2);
}

// I am choosing to test the actual HTTP server integration.
TEST_F(HealthMonitorTest, HttpServerResponse)
{
    auto& monitor = HealthMonitor::Instance();

    monitor.StartServer(k_test_port);

    // I am choosing to use the named constant here
    std::this_thread::sleep_for(k_server_start_up_delay);

    httplib::Client cli("localhost", k_test_port);

    auto res_live = cli.Get("/live");
    ASSERT_TRUE(res_live);
    EXPECT_EQ(res_live->status, 200);
    EXPECT_EQ(res_live->body, "OK");

    monitor.SetComponentStatus("service_a", Status::kHealthy);
    auto res_ready = cli.Get("/ready");
    ASSERT_TRUE(res_ready);
    EXPECT_EQ(res_ready->status, 200);
    EXPECT_EQ(res_ready->body, "READY");

    monitor.SetComponentStatus("service_a", Status::kUnhealthy);
    auto res_unready = cli.Get("/ready");
    ASSERT_TRUE(res_unready);
    EXPECT_EQ(res_unready->status, 503);
    EXPECT_EQ(res_unready->body, "NOT_READY");
}

} // namespace vita::test