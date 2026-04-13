#pragma once

#include <cstddef>
#include <memory>
#include <opentelemetry/sdk/resource/resource.h>
#include <string>
#include <string_view>

#include "vita/config.hpp"

namespace vita::telemetry
{

struct ResourceConfig
{
    std::string_view service_name;
    std::string_view service_version;
};

/**
 * @brief Factory to create an OTel Resource with Vita-specific defaults.
 */
inline auto CreateResource(const ResourceConfig& config) -> ::opentelemetry::sdk::resource::Resource
{
    namespace sdk_resource = ::opentelemetry::sdk::resource;

    // I am choosing to use hardcoded OTel keys to avoid the missing header issue.
    // "service.name" and "service.version" are the standard OTel strings.
    return sdk_resource::Resource::Create({{"service.name", std::string(config.service_name)},
                                           {"service.version", std::string(config.service_version)},
                                           {"deployment.environment", config::GetDeploymentEnv()}});
}

/**
 * @brief RAII Guard for Telemetry life-cycle.
 */
template <typename T> class TelemetryCleanUp
{
public:
    explicit TelemetryCleanUp(std::shared_ptr<T> provider) : provider_(std::move(provider))
    {}

    explicit TelemetryCleanUp(std::nullptr_t) : provider_(nullptr)
    {}

    ~TelemetryCleanUp()
    {
        CleanUp();
    }

    auto CleanUp() -> void
    {
        if (provider_) {
            // I am choosing 'instance' to satisfy the 3-character minimum
            // and clearly represent the object being checked.
            if constexpr (requires(T& instance) {
                              instance.ForceFlush();
                              instance.Shutdown();
                          }) {
                provider_->ForceFlush();
                provider_->Shutdown();
            }
            provider_.reset();
        }
    }

    // Standard RAII: No copying allowed.
    TelemetryCleanUp(const TelemetryCleanUp&) = delete;
    auto operator=(const TelemetryCleanUp&) -> TelemetryCleanUp& = delete;

    // Moving is allowed to transfer ownership.
    TelemetryCleanUp(TelemetryCleanUp&&) noexcept = default;

    // I am choosing to explicitly return TelemetryCleanUp&
    // to satisfy the C++ Core Guidelines and clang-tidy.
    auto operator=(TelemetryCleanUp&&) noexcept -> TelemetryCleanUp& = default;

private:
    std::shared_ptr<T> provider_;
};

} // namespace vita::telemetry