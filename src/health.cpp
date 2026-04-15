#include "vita/health.hpp"

#include <algorithm> // I am choosing to keep this for ranges::all_of
#include <httplib.h>
#include <thread>

namespace vita::health
{

static constexpr int k_http_service_unavailable = 503;

auto HealthMonitor::Instance() -> HealthMonitor&
{
    static HealthMonitor instance;
    return instance;
}

auto HealthMonitor::SetComponentStatus(const std::string& component, Status status) -> void
{
    std::lock_guard<std::mutex> lock(status_mutex_);
    component_statuses_[component] = status;
}

auto HealthMonitor::IsReady() const -> bool
{
    std::lock_guard<std::mutex> lock(status_mutex_);
    if (component_statuses_.empty()) {
        return false;
    }

    // I am choosing to use ranges::all_of to satisfy clang-tidy.
    // It is more expressive and avoids manual iterator management.
    return std::ranges::all_of(component_statuses_, [](const auto& pair) -> bool {
        return pair.second == Status::kHealthy;
    });
}

auto HealthMonitor::StartServer(int port) -> void
{
    std::thread([this, port]() -> void {
        httplib::Server svr;

        svr.Get("/live", [](const httplib::Request&, httplib::Response& res) -> void {
            res.set_content("OK", "text/plain");
        });

        svr.Get("/ready", [this](const httplib::Request&, httplib::Response& res) -> void {
            if (this->IsReady()) {
                res.set_content("READY", "text/plain");
            } else {
                res.status = k_http_service_unavailable;
                res.set_content("NOT_READY", "text/plain");
            }
        });

        svr.listen("0.0.0.0", port);
    }).detach();
}

} // namespace vita::health