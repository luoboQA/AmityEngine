#include <gtest/gtest.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "Entity.hpp"
#include "PanTiltComponent.hpp"

using namespace Core;

// Test 1: PanTiltComponent initialization defaults
TEST(PanTiltComponentTest, InitializationDefaults)
{
    Entity entity;
    auto panTilt = entity.addComponent<PanTiltComponent>();

    ASSERT_NE(panTilt, nullptr);
    EXPECT_FLOAT_EQ(panTilt->getAzimuth(), 0.0f);
    EXPECT_FLOAT_EQ(panTilt->getElevation(), 0.0f);
}

// Test 2: Angle modification and manual set updates
TEST(PanTiltComponentTest, SetAnglesUpdatesRotation)
{
    Entity entity;
    auto panTilt = entity.addComponent<PanTiltComponent>();

    // Set Azimuth and Elevation
    panTilt->setAzimuth(45.0f);
    panTilt->setElevation(30.0f);

    EXPECT_FLOAT_EQ(panTilt->getAzimuth(), 45.0f);
    EXPECT_FLOAT_EQ(panTilt->getElevation(), 30.0f);

    // Compute expected rotation matrix
    glm::mat3 expectedRot = glm::mat3(glm::yawPitchRoll(
        glm::radians(45.0f),
        glm::radians(30.0f),
        0.0f
    ));

    // Verify entity's rotation matches expected
    glm::mat3 actualRot = entity.getRotation();
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            EXPECT_NEAR(actualRot[col][row], expectedRot[col][row], 1e-5f);
        }
    }
}

// Test 3: Setters correctly propagate rotation changes to the owner Entity
TEST(PanTiltComponentTest, RotationUpdatesPropagateCorrectly)
{
    Entity entity;
    auto panTilt = entity.addComponent<PanTiltComponent>();

    // Start at (0, 0)
    EXPECT_FLOAT_EQ(panTilt->getAzimuth(), 0.0f);
    EXPECT_FLOAT_EQ(panTilt->getElevation(), 0.0f);

    // Change azimuth manually
    panTilt->setAzimuth(90.0f);
    EXPECT_FLOAT_EQ(panTilt->getAzimuth(), 90.0f);

    glm::mat3 expectedRot = glm::mat3(glm::yawPitchRoll(
        glm::radians(90.0f),
        glm::radians(0.0f),
        0.0f
    ));

    glm::mat3 actualRot = entity.getRotation();
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            EXPECT_NEAR(actualRot[col][row], expectedRot[col][row], 1e-5f);
        }
    }
}
