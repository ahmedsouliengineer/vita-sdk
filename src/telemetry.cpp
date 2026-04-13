#include "vita/telemetry.hpp"

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

namespace vita::telemetry
{

// I am choosing a modern trailing return type to match your header style.
auto InitializeGlobalTracer(ResourceConfig config)
    -> std::unique_ptr<TelemetryCleanUp<::opentelemetry::trace::TracerProvider>>
{
    namespace trace_sdk = ::opentelemetry::sdk::trace;
    namespace otlp = ::opentelemetry::exporter::otlp;

    // 1. Create the Exporter (Sends data to an OTel Collector)
    auto exporter = otlp::OtlpHttpExporterFactory::Create();

    // 2. Create the Processor (We use Simple for low-latency SDK logic)
    auto processor = trace_sdk::SimpleProcessorFactory::Create(std::move(exporter));

    // 3. Create the Provider with your Resource Factory
    auto resource = CreateResource(config);
    auto provider = trace_sdk::TracerProviderFactory::Create(std::move(processor), resource);

    // 4. Set as Global so the user's app can find it
    ::opentelemetry::trace::Provider::SetTracerProvider(provider);

    return std::make_unique<TelemetryCleanUp<::opentelemetry::trace::TracerProvider>>(provider);
}

} // namespace vita::telemetry