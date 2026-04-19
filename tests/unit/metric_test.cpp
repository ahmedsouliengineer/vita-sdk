#include <gtest/gtest.h>
#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/metrics/provider.h>

#include "vita/metric.hpp"

namespace vita::test
{

namespace
{
// I am choosing to define constants to satisfy cppcoreguidelines-avoid-magic-numbers
constexpr uint64_t k_test_token_count = 50;
constexpr double k_test_latency_value = 0.450;
constexpr double k_basic_latency_value = 1.5;
} // namespace

class MetricTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        vita::metric::InitializeMetrics();
    }
};

TEST_F(MetricTest, InstrumentsAreNonNull)
{
    auto& tokens = vita::metric::LLMTokensCounter();
    auto& latency = vita::metric::TranscriptionLatency();

    // I am choosing to use constants for verification
    EXPECT_NO_THROW(tokens.Add(k_test_token_count));
    EXPECT_NO_THROW(latency.Record(k_basic_latency_value, opentelemetry::context::Context{}));
}

TEST_F(MetricTest, CounterIncrementsSuccessfully)
{
    auto& tokens = vita::metric::LLMTokensCounter();
    EXPECT_NO_THROW(tokens.Add(k_test_token_count));
}

TEST_F(MetricTest, HistogramRecordsSuccessfully)
{
    auto& latency = vita::metric::TranscriptionLatency();

    // I am choosing to use the correct overload for Record.
    // To pass attributes, we often need to provide the context or use a specific initializer.
    std::map<std::string, opentelemetry::common::AttributeValue> attributes = {
        {"operation", "stt"}};

    EXPECT_NO_THROW(
        latency.Record(k_test_latency_value, attributes, opentelemetry::context::Context{}));
}

} // namespace vita::test