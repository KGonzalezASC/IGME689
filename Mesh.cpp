#include "Mesh.h"
#include "Graphics.h"
#include <assimp/postprocess.h>
#include <fstream>
#include <cmath>

// Constructor for non-FBX data remains unchanged.
Mesh::Mesh(const char* name, Vertex* vertexBuffer, int vertexCount, unsigned int* indexBuffer, int indexCount)
    : name(name)
{
    initBuffers(vertexBuffer, vertexCount, indexBuffer, indexCount);
}

// Constructor that loads an FBX file if isFBX is true.
Mesh::Mesh(const char* name, const std::wstring& filePath, bool isFBX) : name(name)
{
    if (isFBX)
    {
        LoadFBX(filePath);
    }
    // Else: implement other loaders as needed.
}

Mesh::~Mesh() {}

// initBuffers creates the vertex and index buffers.
void Mesh::initBuffers(Vertex* vertices, size_t numVerts, unsigned int* indices, size_t numIndices)
{
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = sizeof(Vertex) * (UINT)numVerts;
    vbd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA initialVertexData = {};
    initialVertexData.pSysMem = vertices;
    Graphics::Device->CreateBuffer(&vbd, &initialVertexData, m_vertexBuffer.GetAddressOf());
    m_vertexCount = (UINT)numVerts;

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(unsigned int) * (UINT)numIndices;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA initialIndexData = {};
    initialIndexData.pSysMem = indices;
    Graphics::Device->CreateBuffer(&ibd, &initialIndexData, m_indexBuffer.GetAddressOf());
    m_indicesCount = (UINT)numIndices;
}

// LoadFBX loads the model and animation data using Assimp.
void Mesh::LoadFBX(const std::wstring& filePath)
{
    m_scene = m_importer.ReadFile(std::string(filePath.begin(), filePath.end()),
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_FlipUVs);
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
    {
        // Handle error (e.g., log m_importer.GetErrorString())
        return;
    }

    // Compute the global inverse transform.
    m_GlobalInverseTransform = m_scene->mRootNode->mTransformation;
    m_GlobalInverseTransform.Inverse();

    // Optionally traverse the node hierarchy if needed.
    ProcessNode(m_scene->mRootNode, m_scene);
    LoadAnimations(m_scene);

    // For simplicity, load only the first mesh.
    aiMesh* aMesh = m_scene->mMeshes[0];

    // Create a vertex vector and initialize bone data.
    std::vector<Vertex> vertices;
    vertices.resize(aMesh->mNumVertices);
    for (unsigned int j = 0; j < aMesh->mNumVertices; j++)
    {
        Vertex vertex;
        vertex.Position = XMFLOAT3(aMesh->mVertices[j].x, aMesh->mVertices[j].y, aMesh->mVertices[j].z);
        if (aMesh->mTextureCoords[0])
            vertex.UV = XMFLOAT2(aMesh->mTextureCoords[0][j].x, aMesh->mTextureCoords[0][j].y);
        else
            vertex.UV = XMFLOAT2(0.0f, 0.0f);
        vertex.Normal = XMFLOAT3(aMesh->mNormals[j].x, aMesh->mNormals[j].y, aMesh->mNormals[j].z);
        // Initialize bone data.
        vertex.BoneIndices = XMUINT4(0, 0, 0, 0);
        vertex.BoneWeights = XMFLOAT4(0, 0, 0, 0);
        vertices[j] = vertex;
    }

    // Process bones and assign weights to vertices.
    LoadBones(aMesh, vertices);

    // Process indices.
    std::vector<unsigned int> indices;
    for (unsigned int j = 0; j < aMesh->mNumFaces; j++)
    {
        aiFace face = aMesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; k++)
        {
            indices.push_back(face.mIndices[k]);
        }
    }

    // Create the GPU buffers.
    initBuffers(vertices.data(), vertices.size(), indices.data(), indices.size());

    // Create a constant buffer for bone transforms (support up to 100 bones).
    if (!m_boneBuffer)
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(XMMATRIX) * 100;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        HRESULT hr = Graphics::Device->CreateBuffer(&bd, nullptr, m_boneBuffer.GetAddressOf());
        // Check hr as needed.
    }

    // Resize boneTransforms vector.
    boneTransforms.resize(bones.size(), XMMatrixIdentity());
}

// ProcessNode – can be used to process multiple meshes; here it is minimal.
void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

// ProcessMesh – for further per-mesh processing (not used in this example).
void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    // Optionally extract additional data.
}

// LoadBones assigns bone weights from the aiMesh to the vertices.
void Mesh::LoadBones(aiMesh* mesh, std::vector<Vertex>& vertices)
{
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone* bone = mesh->mBones[i];
        std::string boneName(bone->mName.C_Str());
        unsigned int boneIndex = 0;
        if (boneMapping.find(boneName) == boneMapping.end())
        {
            boneIndex = static_cast<unsigned int>(bones.size());
            boneMapping[boneName] = boneIndex;
            Bone newBone;
            newBone.name = boneName;
            newBone.offsetMatrix = bone->mOffsetMatrix; // store as-is; conversion later
            bones.push_back(newBone);
        }
        else
        {
            boneIndex = boneMapping[boneName];
        }
        // Assign each weight to the corresponding vertex (max 4 influences).
        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            unsigned int vertexId = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            Vertex& v = vertices[vertexId];
            if (v.BoneWeights.x == 0.0f)
            {
                v.BoneIndices.x = boneIndex;
                v.BoneWeights.x = weight;
            }
            else if (v.BoneWeights.y == 0.0f)
            {
                v.BoneIndices.y = boneIndex;
                v.BoneWeights.y = weight;
            }
            else if (v.BoneWeights.z == 0.0f)
            {
                v.BoneIndices.z = boneIndex;
                v.BoneWeights.z = weight;
            }
            else if (v.BoneWeights.w == 0.0f)
            {
                v.BoneIndices.w = boneIndex;
                v.BoneWeights.w = weight;
            }
            // Extra weights are ignored.
        }
    }
}

// LoadAnimations loads basic animation data from the scene.
void Mesh::LoadAnimations(const aiScene* scene)
{
    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* aiAnim = scene->mAnimations[i];
        Animation anim;
        anim.name = aiAnim->mName.C_Str();
        anim.duration = static_cast<float>(aiAnim->mDuration);
        anim.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond ? aiAnim->mTicksPerSecond : 25.0f);
        animations.push_back(anim);
    }
}

// --- Animation Interpolation and Hierarchy Traversal ---

// Converts an aiMatrix4x4 to an XMMATRIX.
XMMATRIX Mesh::AiMatrixToXMMATRIX(const aiMatrix4x4& from)
{
    return XMMATRIX(
        from.a1, from.b1, from.c1, from.d1,
        from.a2, from.b2, from.c2, from.d2,
        from.a3, from.b3, from.c3, from.d3,
        from.a4, from.b4, from.c4, from.d4
    );
}

// Returns the global inverse transform as an XMMATRIX.
XMMATRIX Mesh::GetGlobalInverseMatrix()
{
    return AiMatrixToXMMATRIX(m_GlobalInverseTransform);
}

// Finds the node animation channel for a given node name.
const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
{
    for (unsigned int i = 0; i < animation->mNumChannels; i++)
    {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName)
            return nodeAnim;
    }
    return nullptr;
}

unsigned int Mesh::FindScaling(float animationTime, const aiNodeAnim* nodeAnim)
{
    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
    {
        if (animationTime < (float)nodeAnim->mScalingKeys[i + 1].mTime)
            return i;
    }
    return 0;
}

void Mesh::CalcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim)
{
    if (nodeAnim->mNumScalingKeys == 1)
    {
        out = nodeAnim->mScalingKeys[0].mValue;
        return;
    }
    unsigned int scalingIndex = FindScaling(animationTime, nodeAnim);
    unsigned int nextScalingIndex = scalingIndex + 1;
    float deltaTime = (float)(nodeAnim->mScalingKeys[nextScalingIndex].mTime - nodeAnim->mScalingKeys[scalingIndex].mTime);
    float factor = (animationTime - (float)nodeAnim->mScalingKeys[scalingIndex].mTime) / deltaTime;
    const aiVector3D& start = nodeAnim->mScalingKeys[scalingIndex].mValue;
    const aiVector3D& end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;
    out = start + factor * (end - start);
}

unsigned int Mesh::FindRotation(float animationTime, const aiNodeAnim* nodeAnim)
{
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
    {
        if (animationTime < (float)nodeAnim->mRotationKeys[i + 1].mTime)
            return i;
    }
    return 0;
}

void Mesh::CalcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* nodeAnim)
{
    if (nodeAnim->mNumRotationKeys == 1)
    {
        out = nodeAnim->mRotationKeys[0].mValue;
        return;
    }
    unsigned int rotationIndex = FindRotation(animationTime, nodeAnim);
    unsigned int nextRotationIndex = rotationIndex + 1;
    float deltaTime = (float)(nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
    float factor = (animationTime - (float)nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
    const aiQuaternion& start = nodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
    aiQuaternion::Interpolate(out, start, end, factor);
    out = out.Normalize();
}

unsigned int Mesh::FindPosition(float animationTime, const aiNodeAnim* nodeAnim)
{
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
    {
        if (animationTime < (float)nodeAnim->mPositionKeys[i + 1].mTime)
            return i;
    }
    return 0;
}

void Mesh::CalcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim)
{
    if (nodeAnim->mNumPositionKeys == 1)
    {
        out = nodeAnim->mPositionKeys[0].mValue;
        return;
    }
    unsigned int positionIndex = FindPosition(animationTime, nodeAnim);
    unsigned int nextPositionIndex = positionIndex + 1;
    float deltaTime = (float)(nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
    float factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
    const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D& end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
    out = start + factor * (end - start);
}

// Recursively traverses the node hierarchy to compute each bone’s final transform.
void Mesh::ReadNodeHeirarchy(float animationTime, const aiNode* node, const XMMATRIX& parentTransform)
{
    // Convert the node's transformation matrix.
    XMMATRIX nodeTransformation = AiMatrixToXMMATRIX(node->mTransformation);

    const aiAnimation* animation = m_scene->mAnimations[0];
    const aiNodeAnim* nodeAnim = FindNodeAnim(animation, node->mName.C_Str());
    if (nodeAnim)
    {
        // Interpolate scaling.
        aiVector3D scaling;
        CalcInterpolatedScaling(scaling, animationTime, nodeAnim);
        XMMATRIX scalingM = XMMatrixScaling(scaling.x, scaling.y, scaling.z);

        // Interpolate rotation.
        aiQuaternion rotation;
        CalcInterpolatedRotation(rotation, animationTime, nodeAnim);
        XMVECTOR quat = XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w);
        XMMATRIX rotationM = XMMatrixRotationQuaternion(quat);

        // Interpolate translation.
        aiVector3D translation;
        CalcInterpolatedPosition(translation, animationTime, nodeAnim);
        XMMATRIX translationM = XMMatrixTranslation(translation.x, translation.y, translation.z);

        // Combine interpolated transformations.
        nodeTransformation = scalingM * rotationM * translationM;
    }

    // Compute global transformation for this node.
    XMMATRIX globalTransformation = parentTransform * nodeTransformation;

    // If this node corresponds to a bone, update its final transform.
    std::string nodeName(node->mName.C_Str());
    if (boneMapping.find(nodeName) != boneMapping.end())
    {
        unsigned int boneIndex = boneMapping[nodeName];
        // Final transformation: globalInverse * globalTransformation * boneOffset.
        XMMATRIX globalInverse = GetGlobalInverseMatrix();
        XMMATRIX boneOffset = AiMatrixToXMMATRIX(bones[boneIndex].offsetMatrix);
        XMMATRIX finalTransformation = globalInverse * globalTransformation * boneOffset;
        if (boneIndex < boneTransforms.size())
            boneTransforms[boneIndex] = finalTransformation;
        else
            boneTransforms.push_back(finalTransformation);
    }

    // Recurse for each child.
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(animationTime, node->mChildren[i], globalTransformation);
    }
}

// BoneTransform computes final bone transforms at a given time.
void Mesh::BoneTransform(float time, std::vector<XMMATRIX>& transforms)
{
    // Assume using the first animation.
    const aiAnimation* animation = m_scene->mAnimations[0];
    float ticksPerSecond = animation->mTicksPerSecond != 0 ? (float)animation->mTicksPerSecond : 25.0f;
    float timeInTicks = time * ticksPerSecond;
    float animationTime = fmod(timeInTicks, (float)animation->mDuration);

    // Start recursion from the root node.
    ReadNodeHeirarchy(animationTime, m_scene->mRootNode, XMMatrixIdentity());

    // Return the computed bone transforms.
    transforms = boneTransforms;
}

// UpdateAnimation advances the animation time and updates bone transforms.
void Mesh::UpdateAnimation(float deltaTime)
{
    animationTime += deltaTime;
    BoneTransform(animationTime, boneTransforms);
}

// Draw binds the buffers, updates the bone constant buffer, and issues the draw call.
void Mesh::Draw()
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    Graphics::Context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    Graphics::Context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Update the bone constant buffer with the latest transforms.
    if (!boneTransforms.empty() && m_boneBuffer)
    {
        Graphics::Context->UpdateSubresource(m_boneBuffer.Get(), 0, nullptr, boneTransforms.data(), 0, 0);
        // Bind the bone buffer to slot b3 in the vertex shader.
        Graphics::Context->VSSetConstantBuffers(3, 1, m_boneBuffer.GetAddressOf());
    }

    Graphics::Context->DrawIndexed(m_indicesCount, 0, 0);
}
