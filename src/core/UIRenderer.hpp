#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "Shader.hpp"

namespace Core {

/**
 * @class UIRenderer
 * @brief High-performance, lightweight 2D orthographic UI rendering manager.
 * 
 * The UIRenderer provides an easy-to-use API for drawing panels, text strings, and custom
 * HUD crosshairs. It utilizes dynamic vertex buffer streaming and handles OpenGL state
 * transitions transparently.
 * 
 * ### Coordinate System
 * Screen coordinates are structured as follows:
 * - Origin `(0.0f, 0.0f)` is at the **Top-Left** corner of the screen.
 * - Width increases to the right: `x` goes from `0.0f` to `width`.
 * - Height increases downwards: `y` goes from `0.0f` to `height`.
 * 
 * ### State Management
 * UI elements are rendered during the 2D pass, after all 3D geometry has been rendered.
 * `UIRenderer::Begin()` sets up key states:
 * - Disables depth testing (`GL_DEPTH_TEST`) so UI elements appear on top.
 * - Disables backface culling (`GL_CULL_FACE`) for 2D quad stability.
 * - Enables alpha blending (`GL_BLEND`) with `(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` for opacity/transparency.
 * 
 * `UIRenderer::End()` fully restores previous OpenGL state settings so that the next frame's 
 * 3D scene renders without issues.
 * 
 * ### Initialization
 * Handled automatically by the base `Core::Application` constructor. Custom applications/games 
 * do not need to manually call `Init()` or `Shutdown()`.
 * 
 * ### Basic Usage Example
 * Override `renderUI()` in your `Application` subclass:
 * @code
 * void MyGame::renderUI() override {
 *     // Get current screen dimensions
 *     float w = static_cast<float>(WIDTH);
 *     float h = static_cast<float>(HEIGHT);
 * 
 *     // 1. Draw a semi-transparent dark grey background panel at top-left
 *     // DrawPanel(x, y, width, height, rgbaColor)
 *     glm::vec4 panelBgColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.6f); // 60% opacity
 *     UIRenderer::DrawPanel(10.0f, 10.0f, 300.0f, 120.0f, panelBgColor);
 * 
 *     // 2. Draw text inside the panel
 *     // DrawText(text, x, y, scale, rgbaColor)
 *     UIRenderer::DrawText("TELEMETRY STATS", 20.0f, 20.0f, 1.2f, glm::vec4(1.0f, 0.8f, 0.0f, 1.0f));
 *     UIRenderer::DrawText("FPS: 60", 20.0f, 50.0f, 1.0f, glm::vec4(1.0f));
 *     UIRenderer::DrawText("Score: 999", 20.0f, 80.0f, 1.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
 * 
 *     // 3. Draw central crosshair HUD
 *     UIRenderer::DrawCrosshair(w / 2.0f, h / 2.0f, 20.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
 * }
 * @endcode
 */
class UIRenderer
{
public:
    /**
     * @brief Initializes the UIRenderer, compiles UI shaders, loads fonts, and creates VBO/VAO quads.
     * @note Handled automatically by `Core::Application`.
     */
    static void Init(int width, int height);

    /**
     * @brief Cleans up UI resources (textures, VAOs, VBOs, and shaders).
     * @note Handled automatically by `Core::Application`.
     */
    static void Shutdown();

    /**
     * @brief Adjusts orthographic projections when the screen is resized.
     * @note Handled automatically by `Core::Application`.
     */
    static void SetScreenSize(int width, int height);

    /**
     * @brief Renders a flat, solid, or translucent 2D background panel.
     * @param x Top-left X-coordinate of the panel.
     * @param y Top-left Y-coordinate of the panel.
     * @param w Width of the panel.
     * @param h Height of the panel.
     * @param color Panel color as (Red, Green, Blue, Alpha).
     */
    static void DrawPanel(float x, float y, float w, float h, const glm::vec4& color);

    /**
     * @brief Renders dynamic text strings using the embedded 8x8 bitmap font.
     * @param text The string to render. Supported character set includes standard ASCII and newlines.
     * @param x Starting X-coordinate of the text.
     * @param y Starting Y-coordinate (baseline top) of the text.
     * @param scale Text sizing multiplier (e.g. 1.0f is default, 2.0f is double size).
     * @param color Text color as (Red, Green, Blue, Alpha).
     */
    static void DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color);

    /**
     * @brief Renders a precise, interactive central scanning crosshair.
     * @param centerX Target X center position (typically screen width / 2).
     * @param centerY Target Y center position (typically screen height / 2).
     * @param size Overall outer span/length of the crosshair.
     * @param color Crosshair lines color as (Red, Green, Blue, Alpha).
     */
    static void DrawCrosshair(float centerX, float centerY, float size, const glm::vec4& color);

    /**
     * @brief Sets up OpenGL state, binds UI shaders and buffers once per frame.
     * @note Handled automatically by `Core::Application` before calling `renderUI()`.
     */
    static void Begin();

    /**
     * @brief Restores original OpenGL state and unbinds resources once per frame.
     * @note Handled automatically by `Core::Application` after calling `renderUI()`.
     */
    static void End();

private:
    static void DrawQuad(float x, float y, float w, float h, float u_min, float v_min, float u_max, float v_max, const glm::vec4& color, bool useTexture);

    static unsigned int s_vao;
    static unsigned int s_vbo;
    static unsigned int s_texture;
    static std::shared_ptr<Shader> s_shader;
    static glm::mat4 s_proj;
    static int s_width;
    static int s_height;
    static bool s_initialized;

    // Saved OpenGL states for batching
    static bool s_savedDepthTest;
    static bool s_savedCullFace;
    static bool s_savedBlend;
    static int s_savedBlendSrc;
    static int s_savedBlendDst;
};

} // namespace Core
