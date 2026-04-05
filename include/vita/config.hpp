#pragma once

#include <charconv>
#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace vita::config
{

/**
 * @brief Exception for SDK configuration errors.
 */
class InvalidConfiguration : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Supported Environment Profiles.
 */
enum class EnvProfile : std::uint8_t
{
    Edge,
    Cloud
};

// --- 1. CORE TYPE-SAFE FETCHERS ---

/**
 * @brief Base string fetcher.
 */
template <std::same_as<std::string> T = std::string>
auto GetEnv(std::string_view env_name) -> std::optional<std::string>
{
    const std::string key{env_name};
    const char* const env_p = std::getenv(key.c_str());

    if (env_p == nullptr) {
        return std::nullopt;
    }
    return std::string{env_p};
}

/**
 * @brief Numeric fetcher (Ints, Floats).
 */
template <typename T>
    requires(std::integral<T> && !std::same_as<T, bool>) || std::floating_point<T>
auto GetEnv(std::string_view env_name) -> std::optional<T>
{
    const auto env_str = GetEnv<std::string>(env_name);
    if (!env_str) {
        return std::nullopt;
    }

    T value{};
    const char* first = env_str->data();
    const char* last =
        first + env_str->size(); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // I am choosing to use a suppression here because std::from_chars
    // requires a pointer range, and this is the most efficient way to provide it.
    const auto [ptr, ec] = std::from_chars(first, last, value);

    if (ec == std::errc{}) {
        return value;
    }
    return std::nullopt;
}

/**
 * @brief Boolean fetcher.
 */
template <typename T>
    requires std::same_as<T, bool>
auto GetEnv(std::string_view env_name) -> std::optional<bool>
{
    const auto value = GetEnv<std::string>(env_name);
    if (!value) {
        return std::nullopt;
    }

    // I am choosing to rename 'v' to 'val_view' to satisfy readability-identifier-length.
    const std::string_view val_view{*value};
    if (val_view == "1" || val_view == "true" || val_view == "yes" || val_view == "on") {
        return true;
    }
    if (val_view == "0" || val_view == "false" || val_view == "no" || val_view == "off") {
        return false;
    }

    return std::nullopt;
}

// --- 2. VITA SPECIFIC HELPERS ---

auto GetProfile() -> EnvProfile;
auto GetDeploymentEnv() -> std::string;

} // namespace vita::config