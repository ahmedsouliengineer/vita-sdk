#pragma once

#include <cstddef>
#include <memory>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/resource/semantic_conventions.h>
#include <string>
#include <string_view>

#include "vita/config.hpp"

namespace vita::telemetry
{

/**
 * @brief Configuration for OTel Resource creation.
 * I am choosing this struct to solve 'bugprone-easily-swappable-parameters'.
 * This forces the caller to use designated initializers or an explicit struct,
 * making the 'service_name' and 'service_version' impossible to swap.
 */
struct ResourceConfig
{
    std::string_view service_name;
    std::string_view service_version;
};

/**
 * @brief Factory to create an OTel Resource with Vita-specific defaults.
 * I am choosing the trailing return type '-> opentelemetry::sdk::resource::Resource'
 * to satisfy 'modernize-use-trailing-return-type'.
 */
inline auto CreateResource(ResourceConfig config) -> ::opentelemetry::sdk::resource::Resource
{
    namespace sdk_resource = ::opentelemetry::sdk::resource;

    // I am choosing the global namespace prefix '::' for opentelemetry
    // to resolve any ambiguity with our local 'telemetry' namespace.
    return sdk_resource::Resource::Create(
        {{sdk_resource::SemanticConventions::kServiceName, std::string(config.service_name)},
         {sdk_resource::SemanticConventions::kServiceVersion, std::string(config.service_version)},
         {sdk_resource::SemanticConventions::kDeploymentEnvironment, config::GetDeploymentEnv()}});
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

    /**
     * @brief Manually triggers the provider shutdown.
     */
    auto CleanUp() -> void
    {
        if (provider_) {
            provider_->ForceFlush();
            provider_->Shutdown();
            provider_.reset();
        }
    }

    // Standard RAII: No copying allowed.
    TelemetryCleanUp(const TelemetryCleanUp&) = delete;
    auto operator=(const TelemetryCleanUp&) -> TelemetryCleanUp& = delete;

    // Moving is allowed to transfer ownership.
    TelemetryCleanUp(TelemetryCleanUp&&) noexcept = default;
    auto operator=(TelemetryCleanUp&&) noexcept -> TelemetryCleanUp& = default;

private:
    std::shared_ptr<T> provider_;
};

} // namespace vita::telemetry