#include <gtest/gtest.h>
#include "Event.hpp"
#include <string>

using namespace Core;

// Test 1: Verify simple event registration and triggering
TEST(EventTest, ConnectAndFire)
{
    Event<> myEvent;
    bool triggered = false;

    myEvent.Connect([&triggered]() {
        triggered = true;
    });

    EXPECT_FALSE(triggered);
    myEvent.fire();
    EXPECT_TRUE(triggered);
}

// Test 2: Verify parameter delivery with variadic template
TEST(EventTest, VariadicParameters)
{
    Event<int, float, std::string> myEvent;
    
    int receivedInt = 0;
    float receivedFloat = 0.0f;
    std::string receivedStr = "";

    myEvent.Connect([&](int i, float f, const std::string& s) {
        receivedInt = i;
        receivedFloat = f;
        receivedStr = s;
    });

    myEvent.fire(42, 3.14f, "AmityEngine");

    EXPECT_EQ(receivedInt, 42);
    EXPECT_FLOAT_EQ(receivedFloat, 3.14f);
    EXPECT_EQ(receivedStr, "AmityEngine");
}

// Test 3: Verify multiple independent subscribers
TEST(EventTest, MultipleSubscribers)
{
    Event<int> myEvent;
    
    int sum = 0;
    int product = 1;

    myEvent.Connect([&sum](int value) {
        sum += value;
    });

    myEvent.Connect([&product](int value) {
        product *= value;
    });

    myEvent.fire(5);

    EXPECT_EQ(sum, 5);
    EXPECT_EQ(product, 5);

    myEvent.fire(3);

    EXPECT_EQ(sum, 8);       // 5 + 3 = 8
    EXPECT_EQ(product, 15);   // 5 * 3 = 15
}
