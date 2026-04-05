#include "vita/config.hpp"

#include <cstdlib>
#include <string>

namespace vita::config
{

/**
 * @brief Implementation of GetProfile.
 * I am choosing to use the explicit vita::config prefix to help the linter
 * link this back to the template in the header.
 */
auto GetProfile() -> EnvProfile
{
    // Fetching the VITA_ENV variable.
    // I am choosing to handle "edge" as a specific case for Standard 6.
    const auto profile = vita::config::GetEnv<std::string>("VITA_ENV");

    if (profile.has_value() && (*profile == "edge" || *profile == "EDGE")) {
        return EnvProfile::Edge;
    }

    return EnvProfile::Cloud;
}

/**
 * @brief Implementation of GetDeploymentEnv.
 */
auto GetDeploymentEnv() -> std::string
{
    // I am choosing to use the value_or pattern to ensure we never return an empty string.
    const auto mode = vita::config::GetEnv<std::string>("VITA_DEPLOYMENT_MODE");
    return mode.value_or("dev");
}

} // namespace vita::config