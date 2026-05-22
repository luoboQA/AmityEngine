#include <gtest/gtest.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "FontManager.hpp"

using namespace Core;

// Test 1: Verify Monospace font metrics
TEST(TestUI, FontMetrics)
{
    EXPECT_FLOAT_EQ(FontManager::GetCharWidth(), 8.0f);
    EXPECT_FLOAT_EQ(FontManager::GetCharHeight(), 8.0f);
}

// Test 2: Verify Character UV Coordinate calculations
TEST(TestUI, FontUVMapping)
{
    float u_min, v_min, u_max, v_max;

    // A: Space Character (' ') - index 0, col 0, row 0
    FontManager::GetCharUVs(' ', u_min, v_min, u_max, v_max);
    EXPECT_FLOAT_EQ(u_min, 0.0f);
    EXPECT_FLOAT_EQ(v_min, 0.0f);
    EXPECT_FLOAT_EQ(u_max, 8.0f / 128.0f); // 0.0625
    EXPECT_FLOAT_EQ(v_max, 8.0f / 64.0f);  // 0.125

    // B: Exclamation Character ('!') - index 1, col 1, row 0
    FontManager::GetCharUVs('!', u_min, v_min, u_max, v_max);
    EXPECT_FLOAT_EQ(u_min, 8.0f / 128.0f);
    EXPECT_FLOAT_EQ(v_min, 0.0f);
    EXPECT_FLOAT_EQ(u_max, 16.0f / 128.0f);
    EXPECT_FLOAT_EQ(v_max, 8.0f / 64.0f);

    // Single quote character ('\'') - ASCII 39 -> index 7 -> col 7, row 0
    FontManager::GetCharUVs('\'', u_min, v_min, u_max, v_max);
    EXPECT_FLOAT_EQ(u_min, 56.0f / 128.0f);
    EXPECT_FLOAT_EQ(v_min, 0.0f);
    EXPECT_FLOAT_EQ(u_max, 64.0f / 128.0f);
    EXPECT_FLOAT_EQ(v_max, 8.0f / 64.0f);

    // C: Character '0' - ASCII 48 -> index 16 -> col 0, row 1
    FontManager::GetCharUVs('0', u_min, v_min, u_max, v_max);
    EXPECT_FLOAT_EQ(u_min, 0.0f);
    EXPECT_FLOAT_EQ(v_min, 8.0f / 64.0f);  // 0.125
    EXPECT_FLOAT_EQ(u_max, 8.0f / 128.0f); // 0.0625
    EXPECT_FLOAT_EQ(v_max, 16.0f / 64.0f); // 0.25

    // D: Boundary Character '~' (126) -> index 94 -> col 14, row 5 (94 % 16 = 14, 94 / 16 = 5)
    FontManager::GetCharUVs('~', u_min, v_min, u_max, v_max);
    EXPECT_FLOAT_EQ(u_min, (14 * 8.0f) / 128.0f);
    EXPECT_FLOAT_EQ(v_min, (5 * 8.0f) / 64.0f);
    EXPECT_FLOAT_EQ(u_max, (15 * 8.0f) / 128.0f);
    EXPECT_FLOAT_EQ(v_max, (6 * 8.0f) / 64.0f);

    // E: Out of range low ASCII character (<32) clamps to space (' ')
    FontManager::GetCharUVs(10, u_min, v_min, u_max, v_max); // Newline
    EXPECT_FLOAT_EQ(u_min, 0.0f);
    EXPECT_FLOAT_EQ(v_min, 0.0f);
    EXPECT_FLOAT_EQ(u_max, 0.0625f);
    EXPECT_FLOAT_EQ(v_max, 0.125f);

    // F: Out of range high ASCII character (>126) clamps to space (' ')
    FontManager::GetCharUVs(127, u_min, v_min, u_max, v_max); // DEL
    EXPECT_FLOAT_EQ(u_min, 0.0f);
    EXPECT_FLOAT_EQ(v_min, 0.0f);
    EXPECT_FLOAT_EQ(u_max, 0.0625f);
    EXPECT_FLOAT_EQ(v_max, 0.125f);
}

// Test 3: Verify 2D Orthographic Projection coordinates mapping
TEST(TestUI, OrthographicProjectionMath)
{
    float width = 1280.0f;
    float height = 720.0f;
    
    // Ortho matrix: bottom is 720, top is 0 (y-down coordinate system)
    glm::mat4 proj = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    
    // Top-Left corner (0, 0, 0, 1) -> maps to (-1, 1, 0, 1) in NDC
    glm::vec4 topLeft(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 projectedTopLeft = proj * topLeft;
    EXPECT_FLOAT_EQ(projectedTopLeft.x, -1.0f);
    EXPECT_FLOAT_EQ(projectedTopLeft.y, 1.0f);
    EXPECT_FLOAT_EQ(projectedTopLeft.z, 0.0f);

    // Bottom-Right corner (1280, 720, 0, 1) -> maps to (1, -1, 0, 1) in NDC
    glm::vec4 bottomRight(1280.0f, 720.0f, 0.0f, 1.0f);
    glm::vec4 projectedBottomRight = proj * bottomRight;
    EXPECT_FLOAT_EQ(projectedBottomRight.x, 1.0f);
    EXPECT_FLOAT_EQ(projectedBottomRight.y, -1.0f);
    EXPECT_FLOAT_EQ(projectedBottomRight.z, 0.0f);

    // Middle of screen (640, 360, 0, 1) -> maps to (0, 0, 0, 1) in NDC
    glm::vec4 center(640.0f, 360.0f, 0.0f, 1.0f);
    glm::vec4 projectedCenter = proj * center;
    EXPECT_FLOAT_EQ(projectedCenter.x, 0.0f);
    EXPECT_FLOAT_EQ(projectedCenter.y, 0.0f);
    EXPECT_FLOAT_EQ(projectedCenter.z, 0.0f);
}

// Test 4: Quad vertex layout and coordinate generation math
TEST(TestUI, QuadVerticesGenerationMath)
{
    // Check vertex positions for a panel at (100, 200) with size 50 x 30
    float x = 100.0f;
    float y = 200.0f;
    float w = 50.0f;
    float h = 30.0f;

    // Expected triangle 1 vertices (CCW or CW depending on GL layout)
    // Vertex 0: (x, y)
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y, 200.0f);
    // Vertex 1: (x + w, y)
    EXPECT_FLOAT_EQ(x + w, 150.0f);
    EXPECT_FLOAT_EQ(y, 200.0f);
    // Vertex 2: (x, y + h)
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y + h, 230.0f);

    // Expected triangle 2 vertices
    // Vertex 3: (x, y + h)
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y + h, 230.0f);
    // Vertex 4: (x + w, y)
    EXPECT_FLOAT_EQ(x + w, 150.0f);
    EXPECT_FLOAT_EQ(y, 200.0f);
    // Vertex 5: (x + w, y + h)
    EXPECT_FLOAT_EQ(x + w, 150.0f);
    EXPECT_FLOAT_EQ(y + h, 230.0f);
}
