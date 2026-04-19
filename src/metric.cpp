#include "vita/metric.hpp"

#include <chrono>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>

#include "vita/config.hpp"

namespace vita::metric
{

namespace
{
// Fix: Invalid case style resolved by using kPascalCase for internal constants
constexpr int k_default_export_interval_ms = 60000;

auto GetMeter() -> opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter>
{
    return opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter("vita-sdk", "1.0.0");
}
} // namespace

auto LLMTokensCounter() -> opentelemetry::metrics::Counter<uint64_t>&
{
    static auto counter = GetMeter()->CreateUInt64Counter(
        "vita.llm.tokens", "Total number of tokens processed by the LLM");
    return *counter;
}

auto TranscriptionLatency() -> opentelemetry::metrics::Histogram<double>&
{
    static auto histogram = GetMeter()->CreateDoubleHistogram(
        "vita.stt.latency", "Latency of speech-to-text transcription in seconds");
    return *histogram;
}

auto InitializeMetrics() -> void
{
    // Step 1: Initialize Exporter
    auto exporter = opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create();

    // Step 3: Set up Periodic Reader manually
    auto interval_ms = vita::config::GetEnv<int>("VITA_METRIC_EXPORT_INTERVAL_MS")
                           .value_or(k_default_export_interval_ms);

    // I am choosing the manual constructor path to avoid the missing Factory header
    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options;
    options.export_interval_millis = std::chrono::milliseconds(interval_ms);

    auto reader = std::unique_ptr<opentelemetry::sdk::metrics::MetricReader>(
        new opentelemetry::sdk::metrics::PeriodicExportingMetricReader(std::move(exporter),
                                                                       options));

    // Finalize MeterProvider
    auto provider = opentelemetry::sdk::metrics::MeterProviderFactory::Create();
    auto* sdk_provider = static_cast<opentelemetry::sdk::metrics::MeterProvider*>(provider.get());

    sdk_provider->AddMetricReader(std::move(reader));

    // Register globally with the explicit pointer release pattern
    opentelemetry::metrics::Provider::SetMeterProvider(
        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider>(
            provider.release()));
}

} // namespace vita::metric