#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h> //comptr
#include <fstream>
#include <vector>
#include "Vertex.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <map>

#include "Graphics.h"


//struct BoneInfo {
//    DirectX::XMMATRIX BoneOffset;  // Offset from mesh space to bone space
//    DirectX::XMMATRIX FinalTransformation;
//};

#define NUM_BONES_PER_VEREX 6




class Mesh
{
 private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    unsigned int m_indicesCount;
    unsigned int m_vertexCount;
    const char* name;

    void initBuffers(Vertex* vertices, size_t numVerts, unsigned int* indexArray, size_t numIndices);
    void LoadFBX(const std::wstring& filePath);


    //animation fields
    //std::unordered_map<std::string, unsigned int> mBoneMapping; // Bone name to index
    //std::vector<BoneInfo> mBoneInfo;
    //unsigned int mNumBones = 0;

    //std::vector<DirectX::XMFLOAT4X4> mFinalBoneTransforms;

    void LoadFBX(const std::wstring& filePath);
    void ProcessBoneHierarchy(aiNode* node, DirectX::XMMATRIX parentTransform);
    void ExtractBoneWeightsForVertices(aiMesh* mesh);

    //copy ctor
	//Mesh(const Mesh& mesh);
	////copy assignment
	//Mesh& operator=(const Mesh& mesh);

 public:

     struct VertexBoneData
     {
         unsigned int IDs[NUM_BONES_PER_VEREX];
         float Weights[NUM_BONES_PER_VEREX];
         void AddBoneData(UINT BoneID, float Weight);

     };

     // --------------
// Bone resources
// --------------
     struct BoneInfo
     {
         DirectX::XMMATRIX BoneOffset;
         DirectX::XMMATRIX FinalTransformation;

     };
     struct MeshBoneData
     {
         std::map<std::string, UINT> mBoneMapping; // maps a bone name to its index
         std::vector<BoneInfo> mBoneInfo;
         UINT mNumBones;
         DirectX::XMMATRIX GlobalInverseTransform;
     };

     //stuff from ECS
     struct MeshEntityData
     {
         DirectX::XMFLOAT4X4 worldMatrix;
         DirectX::XMFLOAT3 position;
         DirectX::XMFLOAT3 rotation;
         DirectX::XMFLOAT3 scale;
     };
	//mesh needs device and context to create buffers
    Mesh(const char* name, Vertex* vertexBuffer, int, unsigned int* indexBuffer, int);
    //obj ctor
	Mesh(const char* name, const std::wstring& objFile);

    Mesh(const char* name, const std::wstring& fbxFile, bool isFBX);


    //smart pointers mean destructor can be default
    ~Mesh();

    ID3D11Buffer* GetVertexBuffer() { return m_vertexBuffer.Get(); }
    ID3D11Buffer* GetIndexBuffer() { return m_indexBuffer.Get(); }
    unsigned int GetIndexCount() { return m_indicesCount; }
    unsigned int GetVertexCount() { return m_vertexCount; }
	const char* GetName() { return name; }

    //animation method
    //const std::vector<DirectX::XMFLOAT4X4>& GetFinalBoneTransforms() const { return mFinalBoneTransforms; }
    void LoadBones(aiMesh* mesh, std::vector<VertexBoneData>* BonesData, MeshBoneData* BoneData_ECS);

    void Draw();
};


