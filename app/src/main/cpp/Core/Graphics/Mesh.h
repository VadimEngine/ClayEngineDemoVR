#pragma once
// standard lib
#include <string>
#include <unordered_map>
#include <vector>
// third party
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
// project
#include "Core/Graphics/Shader.h"
#include "XR/XRUtils.h"

class Mesh {
public:
    /**
     * Reads an obj file and populates the given list with the meshes
     *
     * @param path obj file path
     * @param meshList
     */
    static void loadMeshes(AAssetManager* assetManager, const std::string& path, std::vector<Mesh>& meshList);

    /** Mesh Vertex info*/
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
    /** Mesh Texture info*/
    struct Texture {
        unsigned int id;
        std::string type;
        std::string path;
    };

    /** Vertices of this mesh */
    std::vector<Vertex> vertices;
    /** Order to render the vertices in */
    std::vector<unsigned int> indices;
    /** Textures TODO use this*/
    std::vector<Texture> textures;
    /** Vertex Array Object for this mesh */
    unsigned int mVAO;

    /**
     * Constructor
     * @param vertices Vertices for this Mesh
     * @param indices Render order of the vertices
     */
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Destructor
     */
    ~Mesh();

    /**
     * Render the mesh with the given shader. TODO use textures
     * @param theShader Shader that will be used to draw. Textures will be bound to the shader before rendering
     */
    void render(const Shader& theShader) const;

private:
    /**
     * Process a node (and child nodes recursively) in a assimp object and add to
     * @param aiNode Assimp node
     * @param aiScene Assimp scene
     */
    static void processNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshList);

    /**
     * Process an Assimp Mesh and add to list of meshes
     * @param mesh Child node
     * @param scene Assimp Scene
     */
    static Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    /** Initializes the OpenGl properties for this mesh*/
    void buildOpenGLproperties();

    /** Vertex Buffer Object for this mesh*/
    unsigned int mVBO_;
    /** Element Buffer Object for this Mesh */
    unsigned int mEBO_;
};

