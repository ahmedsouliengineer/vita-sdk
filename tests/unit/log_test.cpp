#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include "vita/log.hpp"

namespace vita::test
{

/**
 * @brief Helper to capture clog output for verification.
 * I am choosing to delete copy operations to satisfy CppCoreGuidelines.
 */
class ClogRedirector
{
public:
    explicit ClogRedirector(std::ostream& output_stream)
        : old_buffer_(std::clog.rdbuf(output_stream.rdbuf()))
    {}

    ~ClogRedirector()
    {
        std::clog.rdbuf(old_buffer_);
    }

    // I am choosing to follow the Rule of Five by disabling copying.
    ClogRedirector(const ClogRedirector&) = delete;
    auto operator=(const ClogRedirector&) -> ClogRedirector& = delete;
    ClogRedirector(ClogRedirector&&) = delete;
    auto operator=(ClogRedirector&&) -> ClogRedirector& = delete;

private:
    std::streambuf* old_buffer_;
};

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialization if needed
    }
};

// --- TEST CASES ---

TEST_F(LoggerTest, SingletonInstanceIsSame)
{
    auto& first_instance = vita::log::Logger::Instance();
    auto& second_instance = vita::log::Logger::Instance();
    EXPECT_EQ(&first_instance, &second_instance);
}

TEST_F(LoggerTest, LogWritesToConsoleWhenEnabled)
{
    std::stringstream capture_buffer;
    {
        ClogRedirector redirect(capture_buffer);

        const std::string_view test_message = "Unit test message";
        vita::log::Logger::Instance().Info(test_message);

        const std::string final_output = capture_buffer.str();
        EXPECT_FALSE(final_output.empty());
        EXPECT_NE(final_output.find(test_message), std::string::npos);
        EXPECT_NE(final_output.find("[00000000...]"), std::string::npos);
    }
}

TEST_F(LoggerTest, ConvenienceMethodsCallInternalLog)
{
    std::stringstream capture_buffer;
    {
        ClogRedirector redirect(capture_buffer);

        vita::log::Logger::Instance().Debug("debug_msg");
        vita::log::Logger::Instance().Error("error_msg");

        const std::string final_output = capture_buffer.str();
        EXPECT_NE(final_output.find("debug_msg"), std::string::npos);
        EXPECT_NE(final_output.find("error_msg"), std::string::npos);
    }
}

} // namespace vita::test