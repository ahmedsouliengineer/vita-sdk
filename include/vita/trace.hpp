#pragma once

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/trace/span.h>
#include <string_view>

namespace vita::trace
{

/**
 * @brief RAII wrapper to automatically end spans (Step 3).
 */
class ScopedSpan
{
public:
    explicit ScopedSpan(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span)
        : span_(std::move(span))
    {}

    ~ScopedSpan()
    {
        if (span_) {
            span_->End();
        }
    }

    // Disable copying
    ScopedSpan(const ScopedSpan&) = delete;
    auto operator=(const ScopedSpan&) -> ScopedSpan& = delete;
    ScopedSpan(ScopedSpan&&) = default;
    auto operator=(ScopedSpan&&) -> ScopedSpan& = default;

    auto Get() -> opentelemetry::trace::Span&
    {
        return *span_;
    }

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
};

/**
 * @brief Step 2: Start a span that respects the current RuntimeContext.
 */
auto StartSpan(std::string_view name) -> ScopedSpan;

/**
 * @brief Step 1: Global initialization of the OTLP Exporter.
 */
auto InitializeTracer() -> void;

} // namespace vita::trace