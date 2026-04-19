#pragma once

#include <cstdint>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/sync_instruments.h>

namespace vita::metric
{

/**
 * @brief Global instrument accessors.
 */
auto LLMTokensCounter() -> opentelemetry::metrics::Counter<uint64_t>&;
auto TranscriptionLatency() -> opentelemetry::metrics::Histogram<double>&;

/**
 * @brief Step 1 & 3: Initialization and Export logic.
 */
auto InitializeMetrics() -> void;

} // namespace vita::metric