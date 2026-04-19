#pragma once

#include <cstdint>
#include <opentelemetry/logs/logger.h>
#include <string_view>

namespace vita::log
{

enum class Level : std::uint8_t
{
    kDebug,
    kInfo,
    kWarn,
    kError
};

class Logger
{
public:
    static auto Instance() -> Logger&;

    void Log(Level level, std::string_view message);

    void Debug(std::string_view msg)
    {
        Log(Level::kDebug, msg);
    }
    void Info(std::string_view msg)
    {
        Log(Level::kInfo, msg);
    }
    void Error(std::string_view msg)
    {
        Log(Level::kError, msg);
    }

private:
    Logger();
    opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> otel_logger_;
    bool stdout_enabled_;
};

} // namespace vita::log