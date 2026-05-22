#include "UIRenderer.hpp"
#include "FontManager.hpp"
#include "ResourceManager.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

namespace Core {

// Static member definitions
unsigned int UIRenderer::s_vao = 0;
unsigned int UIRenderer::s_vbo = 0;
unsigned int UIRenderer::s_texture = 0;
std::shared_ptr<Shader> UIRenderer::s_shader = nullptr;
glm::mat4 UIRenderer::s_proj = glm::mat4(1.0f);
int UIRenderer::s_width = 1280;
int UIRenderer::s_height = 720;
bool UIRenderer::s_initialized = false;

// Saved OpenGL states for batching
bool UIRenderer::s_savedDepthTest = false;
bool UIRenderer::s_savedCullFace = false;
bool UIRenderer::s_savedBlend = false;
int UIRenderer::s_savedBlendSrc = 0;
int UIRenderer::s_savedBlendDst = 0;

struct UIVertex {
    glm::vec2 Position;
    glm::vec2 TexCoords;
};

void UIRenderer::Init(int width, int height)
{
    if (s_initialized) return;

    s_width = width;
    s_height = height;
    s_proj = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f); // just ui so not perspective

    // 1. Get or Compile the dedicated 2D UI Shader
    s_shader = ResourceManager::GetShader("UIShader", "src/shaders/ui_vert.glsl", "src/shaders/ui_frag.glsl");

    // 2. Generate and compile font texture
    s_texture = FontManager::CreateFontTexture();

    // 3. Generate VAO and VBO for 2D quad streaming
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    // Allocate space for 6 vertices (1 quad)
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(UIVertex), nullptr, GL_DYNAMIC_DRAW);

    // Positions (Location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoords (Location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, TexCoords));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    s_initialized = true;
    std::cout << "[UIRenderer] Initialized successfully. Resolution: " << width << "x" << height << std::endl;
}

void UIRenderer::Shutdown()
{
    if (!s_initialized) return;

    if (s_vao != 0) glDeleteVertexArrays(1, &s_vao);
    if (s_vbo != 0) glDeleteBuffers(1, &s_vbo);
    if (s_texture != 0) glDeleteTextures(1, &s_texture);

    s_vao = 0;
    s_vbo = 0;
    s_texture = 0;
    s_shader = nullptr;
    s_initialized = false;
}

void UIRenderer::SetScreenSize(int width, int height)
{
    s_width = width;
    s_height = height;
    s_proj = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
}

void UIRenderer::Begin()
{
    if (!s_initialized || !s_shader) return;

    // Save OpenGL states
    s_savedDepthTest = glIsEnabled(GL_DEPTH_TEST);
    s_savedCullFace = glIsEnabled(GL_CULL_FACE);
    s_savedBlend = glIsEnabled(GL_BLEND);
    if (s_savedBlend)
    {
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &s_savedBlendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &s_savedBlendDst);
    }

    // Set UI specific states
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind shader, setup static uniforms & projection once
    s_shader->use();
    s_shader->setMat4("u_Proj", s_proj);

    // Bind VAO and VBO
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);

    // Bind UI Font Texture once
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    s_shader->setInt("u_Texture", 0);
}

void UIRenderer::End()
{
    if (!s_initialized) return;

    // Unbind buffers, vertex array, and texture
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Restore original OpenGL states
    if (s_savedDepthTest) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    if (s_savedCullFace) glEnable(GL_CULL_FACE);
    else glDisable(GL_CULL_FACE);

    if (s_savedBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(s_savedBlendSrc, s_savedBlendDst);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void UIRenderer::DrawQuad(float x, float y, float w, float h, float u_min, float v_min, float u_max, float v_max, const glm::vec4& color, bool useTexture)
{
    if (!s_initialized || !s_shader) return;

    UIVertex vertices[6] = {
        { { x,     y     }, { u_min, v_min } },
        { { x + w, y     }, { u_max, v_min } },
        { { x,     y + h }, { u_min, v_max } },
        
        { { x,     y + h }, { u_min, v_max } },
        { { x + w, y     }, { u_max, v_min } },
        { { x + w, y + h }, { u_max, v_max } }
    };

    s_shader->setVec4("u_Color", color);
    s_shader->setBool("u_UseTexture", useTexture);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UIRenderer::DrawPanel(float x, float y, float w, float h, const glm::vec4& color)
{
    // Draw flat quad with no texture
    DrawQuad(x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color, false);
}

void UIRenderer::DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    float curX = x;
    float curY = y;
    float charW = FontManager::GetCharWidth() * scale;
    float charH = FontManager::GetCharHeight() * scale;

    for (char c : text)
    {
        if (c == '\n')
        {
            curX = x;
            curY += charH + (2.0f * scale); // line spacing
            continue;
        }

        float u_min, v_min, u_max, v_max;
        FontManager::GetCharUVs(c, u_min, v_min, u_max, v_max);

        DrawQuad(curX, curY, charW, charH, u_min, v_min, u_max, v_max, color, true);
        curX += charW;
    }
}

void UIRenderer::DrawCrosshair(float centerX, float centerY, float size, const glm::vec4& color)
{
    float thick = 2.0f; // thickness of lines
    
    // Draw horizontal line
    DrawPanel(centerX - size / 2.0f, centerY - thick / 2.0f, size, thick, color);
    
    // Draw vertical line
    DrawPanel(centerX - thick / 2.0f, centerY - size / 2.0f, thick, size, color);

    // Draw scanning border box notches
    float boxOffset = size / 3.0f;
    float notchLen = size / 6.0f;

    // Top-Left Notch
    DrawPanel(centerX - boxOffset, centerY - boxOffset, notchLen, thick, color);
    DrawPanel(centerX - boxOffset, centerY - boxOffset, thick, notchLen, color);

    // Top-Right Notch
    DrawPanel(centerX + boxOffset - notchLen, centerY - boxOffset, notchLen, thick, color);
    DrawPanel(centerX + boxOffset, centerY - boxOffset, thick, notchLen, color);

    // Bottom-Left Notch
    DrawPanel(centerX - boxOffset, centerY + boxOffset, notchLen, thick, color);
    DrawPanel(centerX - boxOffset, centerY + boxOffset - notchLen, thick, notchLen, color);

    // Bottom-Right Notch
    DrawPanel(centerX + boxOffset - notchLen, centerY + boxOffset, notchLen, thick, color);
    DrawPanel(centerX + boxOffset, centerY + boxOffset - notchLen, thick, notchLen, color);
}

} // namespace Core
