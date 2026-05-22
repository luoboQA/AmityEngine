#include "ModelRenderable.hpp"

// STB IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



// NEW: Helper function to load an embedded texture from a memory buffer
// unsigned int TextureFromMemory(const aiTexture* embeddedTexture)
// {
//     std::cout << "loading texutre from memory: " << embeddedTexture << std::endl;
//     unsigned int textureID;
//     glGenTextures(1, &textureID);

//     int width, height, nrComponents;
//     // Assimp stores compressed data (like a full PNG) in pcData.
//     // mWidth is the size of this data in bytes. mHeight is 0 for compressed.
//     unsigned char* data = stbi_load_from_memory(
//         reinterpret_cast<const stbi_uc*>(embeddedTexture->pcData),
//         embeddedTexture->mWidth,
//         &width, &height, &nrComponents, 0
//     );

//     if (data)
//     {
//         GLenum format;
//         if (nrComponents == 1)
//             format = GL_RED;
//         else if (nrComponents == 3)
//             format = GL_RGB;
//         else if (nrComponents == 4)
//             format = GL_RGBA;

//         glBindTexture(GL_TEXTURE_2D, textureID);
//         glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//         stbi_image_free(data);
//     }
//     else
//     {
//         std::cerr << "Failed to load embedded texture." << std::endl;
//         stbi_image_free(data);
//     }

//     return textureID;
// }



namespace Core {


// MESH
void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    // texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // release VAO
}

Mesh::~Mesh()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
}

void Mesh::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // release VAO
}
// END MESH




// BEGIN MODELRENDERABLE
ModelRenderable::ModelRenderable(const ModelConfig& modelConfig, std::shared_ptr<Shader> shader) : Renderable(shader), m_config(modelConfig)
{
    // m_model = glm::translate(m_model, glm::vec3(0, 200, 0));
    m_scale = m_config.scale;
    m_model = glm::scale(m_model, glm::vec3(m_scale));
    loadModel();
}

void ModelRenderable::loadModel()
{
    const aiScene* scene = m_importer.ReadFile(m_config.modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ASSIMP ERROR: " << m_importer.GetErrorString() << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene);
}

void ModelRenderable::processNode(aiNode *node, const aiScene *scene)
{
    // process all meshes in node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // recurse
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> ModelRenderable::processMesh(aiMesh *mesh, const aiScene *scene)
{
    glm::vec4 materialColor(1.0f);
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        glm::vec3 vector{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            glm::vec3 normal{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
            vertex.Normal = normal;   
        }
        //texture coords
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 textureVec{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            vertex.TexCoords = textureVec;
        }
        else
        {
            vertex.TexCoords = glm::vec2{0.0f, 0.0f};
        }

        vertices.push_back(vertex);
    }

    // faces/indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process materials
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // check base color
        aiColor4D diffuseColor;
        if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor))
        {
            materialColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a) * m_config.brightness;
        }

        // color maps
        // std::vector<Texture> baseColorMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_basecolor", scene);
        // textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());;
        // difuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end()); // append diffsueMaps to txturse
        // specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return std::make_unique<Mesh>(vertices, indices, textures, materialColor);
}

std::vector<Texture> ModelRenderable::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const aiScene* scene)
{
    std::vector<Texture> textures;
    // std::cout << "texture count: " << mat->GetTextureCount(type) << std::endl;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was laoded to prevent duplicates
        bool skip = false;
        for (unsigned int j = 0; j < texturesLoaded.size(); ++j)
        {
            if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Texture texture;
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
            if (embeddedTexture != nullptr)
            {
                // embedded texture
                // texture.id = TextureFromMemory(embeddedTexture); // TODO implement this
            }
            // TODO texture from file

            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture);
        }
    }

    return textures;
}



void ModelRenderable::render(const Scene& scene, double dt)
{
    // m_model = glm::rotate(m_model, static_cast<float>(dt), glm::vec3(0.0f, 1.0f, 0.0f));
    m_shader->use();
    m_shader->setMat4("u_Model", m_model);
    m_shader->setMat4("u_View", scene.getView());
    m_shader->setMat4("u_Proj", scene.getProjection()); // TODO setting view & projection can be redundant cus shaders can be reused across renderables
    
    // CPU Normal Matrix precomputation (extremely high performance gain over vertex shader inverse math)
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_model)));
    m_shader->setMat3("u_NormalMatrix", normalMatrix);

    for (const auto& mesh : meshes)
    {
        // std::cout << "material color: " << mesh->getMaterialColor().y << std::endl;
        m_shader->setVec4("u_MaterialColor", mesh->getMaterialColor());
        mesh->draw();
    }
}


}