// copied from VoxelEngine https://github.com/TheSlabby/VoxelEngine/blob/master/VoxelEngine/Shader.h

#pragma once


#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core {

class Shader
{
public:
	unsigned int ID = 0;
	Shader();
	~Shader();

	// Disable copy constructor and copy assignment to prevent double program deletion
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	// Enable move operations
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	void use() const;
	void setShader(const char* vertexPath, const char* fragmentPath);

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMat3(const std::string& name, glm::mat3 value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
	void setVec2(const std::string& name, glm::vec2 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec4(const std::string& name, glm::vec4 value) const;

};

} // Core namespace