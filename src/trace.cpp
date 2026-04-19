#include "vita/trace.hpp"

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>
#include <vector>

namespace vita::trace
{

auto InitializeTracer() -> void
{
    // Step 1: OTLP Exporter Initialization
    auto exporter = opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create();

    auto processor =
        opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));

    std::vector<std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>> processors;
    processors.push_back(std::move(processor));

    auto sdk_provider =
        opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(processors));

    // Fix: Using the explicit constructor with a released pointer to avoid
    // the ambiguity between unique_ptr and shared_ptr move constructors.
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> api_provider{
        sdk_provider.release()};

    opentelemetry::trace::Provider::SetTracerProvider(api_provider);
}

auto StartSpan(std::string_view name) -> ScopedSpan
{
    auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("vita-sdk");

    opentelemetry::nostd::string_view otel_name{name.data(), name.size()};

    return ScopedSpan{tracer->StartSpan(otel_name)};
}

} // namespace vita::trace