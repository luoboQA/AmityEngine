#pragma once
#include "Renderable.hpp"
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Craft {

enum class BlockType : unsigned char {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    Sand = 4,
    Wood = 5,
    Leaf = 6
};

// Voxel Chunk Dimensions
constexpr int CHUNK_SIZE_X = 32;
constexpr int CHUNK_SIZE_Y = 16;
constexpr int CHUNK_SIZE_Z = 32;
constexpr float BLOCK_SIZE = 2.0f; // Each voxel block is 2x2x2 units

struct VoxelVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class VoxelWorld : public Core::Renderable
{
public:
    VoxelWorld(std::shared_ptr<Core::Shader> shader);
    ~VoxelWorld() override;

    void render(const Core::Scene& scene, double dt) override;

    // Voxel grid operations
    void generateWorld();
    void rebuildMesh();

    bool isSolid(int x, int y, int z) const;
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);

    // Simple raycasting algorithm to find block intersection
    // Returns true if a solid block is hit
    bool raycast(const glm::vec3& origin, const glm::vec3& dir, float maxDist,
                 glm::ivec3& outHitBlock, glm::ivec3& outNeighborBlock) const;

private:
    BlockType m_blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

    // Separate sub-mesh buffers for each block type to batch render colors
    struct BlockSubMesh {
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLsizei VertexCount = 0;
        glm::vec4 Color = glm::vec4(1.0f);
    };

    BlockSubMesh m_subMeshes[7]; // Mapping to indices of BlockType

    void clearSubMeshes();
    void buildSubMesh(BlockType type, const std::vector<VoxelVertex>& vertices);
};

} // namespace Craft
