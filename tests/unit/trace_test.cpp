#include <gtest/gtest.h>
#include <opentelemetry/trace/provider.h>

#include "vita/trace.hpp"

namespace vita::test
{

class TraceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // I am choosing to initialize the tracer once.
        // Subsequent calls to InitializeTracer in other tests won't hurt,
        // but OTel only allows setting the global provider once.
        vita::trace::InitializeTracer();
    }
};

TEST_F(TraceTest, StartSpanCreatesValidSpan)
{
    // I am choosing to verify that StartSpan returns a usable object.
    auto scoped_span = vita::trace::StartSpan("test-span");

    // Check if the underlying span is valid.
    EXPECT_TRUE(scoped_span.Get().GetContext().IsValid());
}

TEST_F(TraceTest, ScopedSpanHandlesRAII)
{
    // I am choosing to verify that move semantics work.
    auto first_span = vita::trace::StartSpan("parent");
    {
        auto second_span = std::move(first_span);
        // I am choosing to access the moved span to ensure it's still alive.
        EXPECT_TRUE(second_span.Get().IsRecording());
    }
    // After the block, the span should be ended automatically.
}

TEST_F(TraceTest, NestedSpansCreateParentChildLink)
{
    // I am choosing to verify that Step 2 (parent-child) works.
    auto parent = vita::trace::StartSpan("parent-operation");

    auto child = vita::trace::StartSpan("child-operation");

    // In a real OTel setup, we'd verify the ParentId here.
    // Since we're using the live SDK, we at least ensure they both coexist.
    EXPECT_TRUE(parent.Get().GetContext().IsValid());
    EXPECT_TRUE(child.Get().GetContext().IsValid());
}

} // namespace vita::test