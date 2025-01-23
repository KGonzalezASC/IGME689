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


#include "Graphics.h"




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

    //copy ctor
	//Mesh(const Mesh& mesh);
	////copy assignment
	//Mesh& operator=(const Mesh& mesh);

 public:
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

    void Draw();
};


