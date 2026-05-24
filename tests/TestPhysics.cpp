#include <gtest/gtest.h>
#include "Entity.hpp"
#include "RigidBodyComponent.hpp"

using namespace Core;

// Test 1: Verify RigidBodyComponent defaults
TEST(PhysicsTest, DefaultValues)
{
    Entity entity;
    auto rb = entity.addComponent<RigidBodyComponent>();

    ASSERT_NE(rb, nullptr);
    EXPECT_FLOAT_EQ(rb->getVelocity().x, 0.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().y, 0.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().z, 0.0f);

    EXPECT_FLOAT_EQ(rb->getAcceleration().x, 0.0f);
    EXPECT_FLOAT_EQ(rb->getAcceleration().y, 0.0f);
    EXPECT_FLOAT_EQ(rb->getAcceleration().z, 0.0f);
}

// Test 2: Verify translation integration based on velocity
TEST(PhysicsTest, VelocityIntegration)
{
    Entity entity;
    entity.setPosition({10.0f, 20.0f, 30.0f});

    auto rb = entity.addComponent<RigidBodyComponent>();
    rb->setVelocity({2.0f, -1.0f, 5.0f});

    // Run one update step (dt = 0.5s)
    entity.update(0.5);

    // Expected position: pos + vel * dt -> {10+1, 20-0.5, 30+2.5}
    EXPECT_FLOAT_EQ(entity.getPosition().x, 11.0f);
    EXPECT_FLOAT_EQ(entity.getPosition().y, 19.5f);
    EXPECT_FLOAT_EQ(entity.getPosition().z, 32.5f);
}

// Test 3: Verify acceleration integration updating velocity
TEST(PhysicsTest, AccelerationIntegration)
{
    Entity entity;
    auto rb = entity.addComponent<RigidBodyComponent>();
    rb->setAcceleration({0.0f, -10.0f, 0.0f}); // 10 m/s^2 down

    // Step 1 (dt = 0.1s)
    entity.update(0.1);
    EXPECT_FLOAT_EQ(rb->getVelocity().x, 0.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().y, -1.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().z, 0.0f);

    // Step 2 (dt = 0.4s)
    entity.update(0.4);
    EXPECT_FLOAT_EQ(rb->getVelocity().x, 0.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().y, -5.0f);
    EXPECT_FLOAT_EQ(rb->getVelocity().z, 0.0f);
}

// Test 4: Verify complex parabolic trajectory integration
TEST(PhysicsTest, ParabolicTrajectory)
{
    Entity entity;
    entity.setPosition({0.0f, 0.0f, 0.0f});

    auto rb = entity.addComponent<RigidBodyComponent>();
    rb->setVelocity({10.0f, 20.0f, 0.0f});     // Initial velocity: 10 right, 20 up
    rb->setAcceleration({0.0f, -10.0f, 0.0f});  // Gravity: -10 m/s^2 down

    // Integrate over multiple steps (4 steps of 0.5 seconds each)
    // Step 1: dt = 0.5
    entity.update(0.5);
    // vel_y = 20 - 5 = 15; pos_y = 0 + 20*0.5 = 10
    EXPECT_FLOAT_EQ(rb->getVelocity().y, 15.0f);
    EXPECT_FLOAT_EQ(entity.getPosition().y, 10.0f);

    // Step 2: dt = 0.5
    entity.update(0.5);
    // vel_y = 15 - 5 = 10; pos_y = 10 + 15*0.5 = 17.5
    EXPECT_FLOAT_EQ(rb->getVelocity().y, 10.0f);
    EXPECT_FLOAT_EQ(entity.getPosition().y, 17.5f);

    // Step 3: dt = 0.5
    entity.update(0.5);
    // vel_y = 10 - 5 = 5; pos_y = 17.5 + 10*0.5 = 22.5
    EXPECT_FLOAT_EQ(rb->getVelocity().y, 5.0f);
    EXPECT_FLOAT_EQ(entity.getPosition().y, 22.5f);

    // Step 4: dt = 0.5
    entity.update(0.5);
    // vel_y = 5 - 5 = 0; pos_y = 22.5 + 5*0.5 = 25 (reaches peak!)
    EXPECT_FLOAT_EQ(rb->getVelocity().y, 0.0f);
    EXPECT_FLOAT_EQ(entity.getPosition().y, 25.0f);

    // Check final X position: vel_x * total_t -> 10 * 2.0 = 20.0
    EXPECT_FLOAT_EQ(entity.getPosition().x, 20.0f);
}
