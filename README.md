# AmityEngine

A lightweight 3D game engine written in C++23 with OpenGL

![CI](https://github.com/TheSlabby/AmityEngine/actions/workflows/cmake-tests.yml/badge.svg)

---

## Screenshots

<img width="1669" height="660" alt="image" src="https://github.com/user-attachments/assets/9495fab6-0e01-4bb5-ad7b-47579c12a5b7" />
<img width="1661" height="680" alt="image" src="https://github.com/user-attachments/assets/66635eaa-6c35-43eb-b097-e4f55d8ef6e2" />
<img width="1646" height="873" alt="image" src="https://github.com/user-attachments/assets/357fb50f-3a6c-4052-b8e1-ea1ddbe109eb" />
<img width="1202" height="825" alt="image" src="https://github.com/user-attachments/assets/6492c8bb-b566-4652-9d99-7d2cd058e5d9" />


---

## Engine Features/Architecture

- **Actor-Component Model** — OOP-style `Entity`, `Component`, and `Scene`
- **3D Model Loading** — Assimp for 3D model loading
- **Terrain Generation** — Simple terrain with diffuse lighting
- **Water Rendering** — Real-time animated ocean surface via fragment shader
- **Spatial Audio** — Using OpenAL
- **Font Rendering** — Font bitmap for font rendering
- **UI System** — Very simple UI system (panels, text)
- **Lua Scripting** — Embedded LUA (still very early)
- **Pan/Tilt Camera** — For azimuth/elevation camera pointing
- **Simple Shader Pipeline**
- **Resource Caching** — With singleton `ResourceManager`

---


## Example Code Snippets

```cpp

// 1. Create scene
auto scene = std::make_shared<Core::Scene>();

// 2. Use ResourceManager to load shaders & models
auto shader = Core::ResourceManager::GetShader("main_shader", "vert.glsl", "frag.glsl");
auto model  = Core::ResourceManager::GetModel("assets/models/ship.obj", 1.0f, 1.0f, shader);

// 3. Create Entity & move it
auto entity = std::make_shared<Core::Entity>();
entity->setName("PirateShip");
entity->setPosition(glm::vec3(0.0f, 5.0f, -10.0f));
entity->setScale(glm::vec3(2.5f));

// 4. Add a component to the entity
// Components basically add behavior to an entity
entity->addComponent<Core::MeshComponent>(model, shader);

// 5. Add entity to the scene (so it can be rendered and interacted with)
scene->addEntity(entity);
```

---



## Building

**Dependencies**: OpenGL, GLFW, GLM, Assimp, OpenAL, libsndfile

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Run tests:**

```bash
cd build && ctest -C Debug --output-on-failure
```

---

## Shader References

| Shader | Source |
|--------|--------|
| Water Fragment (`waterFrag.glsl`) | [Shadertoy — MdXyzX](https://www.shadertoy.com/view/MdXyzX) |
| Volumetric Clouds (`postprocessFrag.glsl`) | [Shadertoy — XtBXDw](https://www.shadertoy.com/view/XtBXDw) by valentingalea |
