#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

namespace vita::health
{

enum class Status : std::uint8_t
{
    kStarting,
    kHealthy,
    kDegraded,
    kUnhealthy
};

/**
 * @brief I am choosing a Singleton approach for Health to ensure
 * probes always hit the single source of truth for the process.
 */
class HealthMonitor
{
public:
    // I am choosing a named constant to avoid magic numbers.
    static constexpr int k_default_port = 8080;

    static auto Instance() -> HealthMonitor&;

    // I am choosing to let different components register their readiness.
    void SetComponentStatus(const std::string& component, Status status);

    auto IsLive() const -> bool;  // Returns true if process is running
    auto IsReady() const -> bool; // Returns true if all critical components are kHealthy

    // I am choosing to start the server in a non-blocking background thread.
    auto StartServer(int port = k_default_port) -> void;
    auto StopServer() -> void;

private:
    HealthMonitor() : is_running_(true)
    {}
    std::atomic<bool> is_running_;
    std::map<std::string, Status> component_statuses_;
    mutable std::mutex status_mutex_;
};

} // namespace vita::health