#include "vita/telemetry.hpp"

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

namespace vita::telemetry
{

auto InitializeGlobalTracer(const ResourceConfig& config)
    -> std::unique_ptr<TelemetryCleanUp<::opentelemetry::sdk::trace::TracerProvider>>
{
    namespace trace_sdk = ::opentelemetry::sdk::trace;
    namespace otlp = ::opentelemetry::exporter::otlp; // Fix: Undeclared identifier

    auto exporter = otlp::OtlpHttpExporterFactory::Create();
    auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
    auto resource = CreateResource(config);

    auto sdk_provider_uniq =
        trace_sdk::TracerProviderFactory::Create(std::move(processor), resource);

    // We keep std::shared_ptr for our Vita guard
    auto sdk_provider = std::shared_ptr<trace_sdk::TracerProvider>(std::move(sdk_provider_uniq));

    // Fix: Explicitly wrap std::shared_ptr in nostd::shared_ptr for the Global Provider
    ::opentelemetry::trace::Provider::SetTracerProvider(
        ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::TracerProvider>(sdk_provider));

    return std::make_unique<TelemetryCleanUp<trace_sdk::TracerProvider>>(sdk_provider);
}

} // namespace vita::telemetry