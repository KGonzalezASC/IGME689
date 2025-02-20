#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "Vertex.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Graphics.h"

using namespace DirectX;

// Structure for a single bone
struct Bone {
    std::string name;
    aiMatrix4x4 offsetMatrix; // stored in Assimp format; convert when needed
};

// Structure for an animation clip (basic data)
struct Animation {
    std::string name;
    float duration;
    float ticksPerSecond;
};

class Mesh
{
private:
    // GPU buffers
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    unsigned int m_indicesCount;
    unsigned int m_vertexCount;
    const char* name;

    // Initialize buffers
    void initBuffers(Vertex* vertices, size_t numVerts, unsigned int* indexArray, size_t numIndices);

    // Animation support members
    std::vector<Bone> bones;
    std::unordered_map<std::string, unsigned int> boneMapping; // maps bone name to bone index
    std::vector<Animation> animations;
    float animationTime = 0.0f;
    std::vector<XMMATRIX> boneTransforms; // final bone transformation matrices
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_boneBuffer; // constant buffer for bone transforms

    // Members to support proper keyframe interpolation and hierarchy traversal:
    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;
    aiMatrix4x4 m_GlobalInverseTransform;

    // Helper functions for FBX processing:
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
    // Revised LoadBones that also updates the vertices’ skinning data.
    void LoadBones(aiMesh* mesh, std::vector<Vertex>& vertices);
    void LoadAnimations(const aiScene* scene);

    // Animation interpolation and hierarchy traversal:
    void BoneTransform(float time, std::vector<XMMATRIX>& transforms);
    void ReadNodeHeirarchy(float animationTime, const aiNode* node, const XMMATRIX& parentTransform);
    const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
    void CalcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* nodeAnim);
    void CalcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim);
    unsigned int FindScaling(float animationTime, const aiNodeAnim* nodeAnim);
    unsigned int FindRotation(float animationTime, const aiNodeAnim* nodeAnim);
    unsigned int FindPosition(float animationTime, const aiNodeAnim* nodeAnim);

    // Helper to convert an aiMatrix4x4 to an XMMATRIX.
    XMMATRIX AiMatrixToXMMATRIX(const aiMatrix4x4& from);

    // Helper to get global inverse transform as an XMMATRIX.
    XMMATRIX GetGlobalInverseMatrix();

public:
    // Loads an FBX file (with embedded animations)
    void LoadFBX(const std::wstring& filePath);

    // Advances the animation and updates bone transforms.
    void UpdateAnimation(float deltaTime);

    // Constructors
    Mesh(const char* name, Vertex* vertexBuffer, int vertexCount, unsigned int* indexBuffer, int indexCount);
    Mesh(const char* name, const std::wstring& fbxFile, bool isFBX);

    ~Mesh();

    ID3D11Buffer* GetVertexBuffer() { return m_vertexBuffer.Get(); }
    ID3D11Buffer* GetIndexBuffer() { return m_indexBuffer.Get(); }
    unsigned int GetIndexCount() { return m_indicesCount; }
    unsigned int GetVertexCount() { return m_vertexCount; }
    const char* GetName() { return name; }

    void Draw();
};