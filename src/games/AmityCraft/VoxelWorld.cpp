#include "VoxelWorld.hpp"
#include "Scene.hpp"
#include "stb_perlin.h"
#include <iostream>
#include <cmath>

namespace Craft {

// Define unit quad vertices for all 6 faces of a cube
// Position offsets relative to block origin (0, 0, 0)
// Face normals point outwards

// Front face (+Z)
static const VoxelVertex frontFace[6] = {
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
};

// Back face (-Z)
static const VoxelVertex backFace[6] = {
    { { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }
};

// Left face (-X)
static const VoxelVertex leftFace[6] = {
    { { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
    { { 0.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } }
};

// Right face (+X)
static const VoxelVertex rightFace[6] = {
    { { 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } }
};

// Top face (+Y)
static const VoxelVertex topFace[6] = {
    { { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }
};

// Bottom face (-Y)
static const VoxelVertex bottomFace[6] = {
    { { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
    { { 1.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } }
};

VoxelWorld::VoxelWorld(std::shared_ptr<Core::Shader> shader) : Core::Renderable(shader)
{
    // Define material colors for each block type in the mesh batching
    m_subMeshes[0].Color = glm::vec4(0.0f); // Air (Not drawn)
    m_subMeshes[1].Color = glm::vec4(0.18f, 0.55f, 0.16f, 1.0f); // Grass (forest green)
    m_subMeshes[2].Color = glm::vec4(0.48f, 0.33f, 0.18f, 1.0f); // Dirt (sand brown)
    m_subMeshes[3].Color = glm::vec4(0.50f, 0.50f, 0.50f, 1.0f); // Stone (granite grey)
    m_subMeshes[4].Color = glm::vec4(0.85f, 0.80f, 0.45f, 1.0f); // Sand (desert gold)
    m_subMeshes[5].Color = glm::vec4(0.38f, 0.25f, 0.12f, 1.0f); // Wood (bark brown)
    m_subMeshes[6].Color = glm::vec4(0.08f, 0.40f, 0.08f, 1.0f); // Leaf (dark green)

    generateWorld();
    rebuildMesh();
}

VoxelWorld::~VoxelWorld()
{
    clearSubMeshes();
}

void VoxelWorld::clearSubMeshes()
{
    for (int i = 1; i <= 6; ++i)
    {
        if (m_subMeshes[i].VAO != 0)
        {
            glDeleteVertexArrays(1, &m_subMeshes[i].VAO);
            m_subMeshes[i].VAO = 0;
        }
        if (m_subMeshes[i].VBO != 0)
        {
            glDeleteBuffers(1, &m_subMeshes[i].VBO);
            m_subMeshes[i].VBO = 0;
        }
        m_subMeshes[i].VertexCount = 0;
    }
}

bool VoxelWorld::isSolid(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
    {
        return false; // Out of bounds are non-solid
    }
    return m_blocks[x][y][z] != BlockType::Air;
}

BlockType VoxelWorld::getBlock(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z)
    {
        return BlockType::Air;
    }
    return m_blocks[x][y][z];
}

void VoxelWorld::setBlock(int x, int y, int z, BlockType type)
{
    if (x >= 0 && x < CHUNK_SIZE_X && y >= 0 && y < CHUNK_SIZE_Y && z >= 0 && z < CHUNK_SIZE_Z)
    {
        m_blocks[x][y][z] = type;
    }
}

void VoxelWorld::generateWorld()
{
    // 1. Core Terrain Layout (Perlin Noise)
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z)
        {
            // Sample terrain height smoothly
            float noise = stb_perlin_noise3(x * 0.06f, z * 0.06f, 0.0f, 0, 0, 0);
            int height = static_cast<int>(5.0f + 6.0f * (noise + 1.0f) * 0.5f);
            if (height >= CHUNK_SIZE_Y) height = CHUNK_SIZE_Y - 1;
            if (height < 0) height = 0;

            for (int y = 0; y < CHUNK_SIZE_Y; ++y)
            {
                if (y < height - 3)
                {
                    m_blocks[x][y][z] = BlockType::Stone;
                }
                else if (y < height)
                {
                    m_blocks[x][y][z] = BlockType::Dirt;
                }
                else if (y == height)
                {
                    if (height <= 3)
                    {
                        m_blocks[x][y][z] = BlockType::Sand; // Water level sandy beaches
                    }
                    else
                    {
                        m_blocks[x][y][z] = BlockType::Grass;
                    }
                }
                else
                {
                    m_blocks[x][y][z] = BlockType::Air;
                }
            }
        }
    }

    // 2. Tree Scattering
    // Coordinates picked to look scattered and organic
    std::vector<glm::ivec2> treeCoords = {
        {6, 6}, {12, 22}, {22, 10}, {26, 26}, {15, 15}
    };

    for (const auto& coord : treeCoords)
    {
        int x = coord.x;
        int z = coord.y;

        // Find the grass level
        int grassY = -1;
        for (int y = CHUNK_SIZE_Y - 1; y >= 0; --y)
        {
            if (m_blocks[x][y][z] == BlockType::Grass)
            {
                grassY = y;
                break;
            }
        }

        if (grassY != -1 && grassY < CHUNK_SIZE_Y - 5)
        {
            // Trunk: 3 Blocks High
            m_blocks[x][grassY + 1][z] = BlockType::Wood;
            m_blocks[x][grassY + 2][z] = BlockType::Wood;
            m_blocks[x][grassY + 3][z] = BlockType::Wood;

            // Canopy: 3x3x2 leaves on top
            for (int lx = -1; lx <= 1; ++lx)
            {
                for (int lz = -1; lz <= 1; ++lz)
                {
                    for (int ly = 3; ly <= 4; ++ly)
                    {
                        int pX = x + lx;
                        int pY = grassY + ly;
                        int pZ = z + lz;

                        if (pX >= 0 && pX < CHUNK_SIZE_X && pY >= 0 && pY < CHUNK_SIZE_Y && pZ >= 0 && pZ < CHUNK_SIZE_Z)
                        {
                            // Place leaves, but don't overwrite the trunk itself
                            if (!(lx == 0 && lz == 0 && ly == 3))
                            {
                                m_blocks[pX][pY][pZ] = BlockType::Leaf;
                            }
                        }
                    }
                }
            }
            // Peak leaf block
            if (grassY + 5 < CHUNK_SIZE_Y)
            {
                m_blocks[x][grassY + 5][z] = BlockType::Leaf;
            }
        }
    }
}

void VoxelWorld::rebuildMesh()
{
    clearSubMeshes();

    // Vertices lists for each block type (1 to 6)
    std::vector<VoxelVertex> meshVertices[7];

    // Build the vertex buffers using dynamic face culling
    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z)
            {
                BlockType type = m_blocks[x][y][z];
                if (type == BlockType::Air) continue;

                int typeIdx = static_cast<int>(type);

                // Absolute position offset in world coordinates
                glm::vec3 posOffset(x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE);

                auto addFace = [&](const VoxelVertex* faceTemplate) {
                    for (int f = 0; f < 6; ++f)
                    {
                        VoxelVertex v = faceTemplate[f];
                        // Scale and offset vertices
                        v.Position = v.Position * BLOCK_SIZE + posOffset;
                        meshVertices[typeIdx].push_back(v);
                    }
                };

                // Front face (+Z)
                if (!isSolid(x, y, z + 1)) addFace(frontFace);

                // Back face (-Z)
                if (!isSolid(x, y, z - 1)) addFace(backFace);

                // Left face (-X)
                if (!isSolid(x - 1, y, z)) addFace(leftFace);

                // Right face (+X)
                if (!isSolid(x + 1, y, z)) addFace(rightFace);

                // Top face (+Y)
                if (!isSolid(x, y + 1, z)) addFace(topFace);

                // Bottom face (-Y)
                if (!isSolid(x, y - 1, z)) addFace(bottomFace);
            }
        }
    }

    // Upload vertices to OpenGL for each sub-mesh
    for (int i = 1; i <= 6; ++i)
    {
        if (!meshVertices[i].empty())
        {
            buildSubMesh(static_cast<BlockType>(i), meshVertices[i]);
        }
    }
}

void VoxelWorld::buildSubMesh(BlockType type, const std::vector<VoxelVertex>& vertices)
{
    int idx = static_cast<int>(type);
    m_subMeshes[idx].VertexCount = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &m_subMeshes[idx].VAO);
    glGenBuffers(1, &m_subMeshes[idx].VBO);

    glBindVertexArray(m_subMeshes[idx].VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_subMeshes[idx].VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelVertex), vertices.data(), GL_STATIC_DRAW);

    // Positions (Location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals (Location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, Normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VoxelWorld::render(const Core::Scene& scene, double dt)
{
    m_shader->use();
    m_shader->setMat4("u_Model", m_model);
    m_shader->setMat4("u_View", scene.getView());
    m_shader->setMat4("u_Proj", scene.getProjection());

    // High performance normal matrix precomputation
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_model)));
    m_shader->setMat3("u_NormalMatrix", normalMatrix);

    // Draw all non-empty sub-meshes
    for (int i = 1; i <= 6; ++i)
    {
        if (m_subMeshes[i].VertexCount > 0)
        {
            m_shader->setVec4("u_MaterialColor", m_subMeshes[i].Color);
            glBindVertexArray(m_subMeshes[i].VAO);
            glDrawArrays(GL_TRIANGLES, 0, m_subMeshes[i].VertexCount);
        }
    }

    glBindVertexArray(0);
}

bool VoxelWorld::raycast(const glm::vec3& origin, const glm::vec3& dir, float maxDist,
                         glm::ivec3& outHitBlock, glm::ivec3& outNeighborBlock) const
{
    // Fast incremental ray marching to find block intersection
    float step = 0.05f;
    glm::vec3 currentPos = origin;
    glm::vec3 dirNorm = glm::normalize(dir);

    glm::ivec3 prevBlock(-1);

    for (float dist = 0.0f; dist < maxDist; dist += step)
    {
        currentPos = origin + dirNorm * dist;

        int vx = static_cast<int>(std::floor(currentPos.x / BLOCK_SIZE));
        int vy = static_cast<int>(std::floor(currentPos.y / BLOCK_SIZE));
        int vz = static_cast<int>(std::floor(currentPos.z / BLOCK_SIZE));

        glm::ivec3 currBlock(vx, vy, vz);

        // Check if inside bounds
        if (vx >= 0 && vx < CHUNK_SIZE_X && vy >= 0 && vy < CHUNK_SIZE_Y && vz >= 0 && vz < CHUNK_SIZE_Z)
        {
            if (m_blocks[vx][vy][vz] != BlockType::Air)
            {
                // Hit solid block!
                outHitBlock = currBlock;
                if (prevBlock != currBlock && prevBlock.x != -1)
                {
                    outNeighborBlock = prevBlock;
                }
                else
                {
                    // Fallback to entry face based on normal or step direction
                    outNeighborBlock = currBlock;
                }
                return true;
            }
        }
        else
        {
            // Left chunk bounds, terminate raycast
            if (dist > 1.0f) break;
        }

        prevBlock = currBlock;
    }

    return false;
}

} // namespace Craft
