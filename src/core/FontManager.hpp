#pragma once

namespace Core {

class FontManager
{
public:
    // Generates a 128x64 1-channel GL texture containing the 95 printable ASCII glyphs.
    // Call within an active OpenGL context.
    static unsigned int CreateFontTexture();

    // Retrieves the exact UV texture coordinates for a given ASCII character (32 to 126).
    static void GetCharUVs(char c, float& u_min, float& v_min, float& u_max, float& v_max);

    // Returns font metrics
    static float GetCharWidth() { return 8.0f; }
    static float GetCharHeight() { return 8.0f; }
};

} // namespace Core
