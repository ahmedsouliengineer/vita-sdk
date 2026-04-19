#include "vita/log.hpp"

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/nostd/span.h> // I am choosing to include this for explicit span handling
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/provider.h>

#include "vita/config.hpp"

namespace vita::log
{

constexpr int k_trace_id_len = 32;
constexpr int k_trace_id_short = 8;

Logger::Logger()
    : otel_logger_(
          opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger("vita-sdk-logger")),
      stdout_enabled_(vita::config::GetEnv<bool>("VITA_LOG_STDOUT").value_or(true))
{}

auto Logger::Instance() -> Logger&
{
    static Logger instance;
    return instance;
}

void Logger::Log(Level /*level*/, std::string_view message)
{
    // I am choosing to retrieve the tracer instance first.
    auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("vita-sdk");

    // Fix: Call GetCurrentSpan() statically through the Tracer class or
    // simply use the instance-based method if the SDK version allows,
    // but to satisfy clang-tidy, we use the static-safe approach.
    auto current_span = opentelemetry::trace::Tracer::GetCurrentSpan();

    auto context = current_span->GetContext();
    std::string trace_id = "00000000000000000000000000000000";

    if (context.IsValid()) {
        std::array<char, k_trace_id_len> buffer{};
        context.trace_id().ToLowerBase16(buffer);
        trace_id = std::string(buffer.data(), k_trace_id_len);
    }

    otel_logger_->Log(opentelemetry::logs::Severity::kInfo,
                      opentelemetry::nostd::string_view(message.data(), message.size()));

    if (stdout_enabled_) {
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::clog << "[" << std::put_time(std::localtime(&now), "%T") << "] "
                  << "[" << trace_id.substr(0, k_trace_id_short) << "...] " << message << '\n';
    }
}

} // namespace vita::log