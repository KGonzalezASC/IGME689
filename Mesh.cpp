#include "Mesh.h"
#include "Graphics.h"

using namespace DirectX;
//implement header / interface

Vertex vertex;
int count = 0; // count vertices

//---------------------------------------
	// - ECS Data stuff in an attempt at animation
	//---------------------------------------
DirectX::XMFLOAT4X4 temp_wm;
MeshEntityData meshEntity = MeshEntityData(temp_wm, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
std::map<std::string, UINT> mBoneMappingTemp;
std::vector<BoneInfo> mBoneInfo;
DirectX::XMMATRIX temp_NodeTransform = DirectX::XMMatrixIdentity();

MeshBoneData* meshBoneData = new MeshBoneData(mBoneMappingTemp, mBoneInfo, UINT(0), temp_NodeTransform);

//ctor
Mesh::Mesh(const char* name, Vertex* vertexBuffer, int vertexCount, unsigned int* indexBuffer, int indexCount): name(name)
{
	DirectX::XMStoreFloat4x4(&temp_wm, DirectX::XMMatrixIdentity());
	initBuffers(vertexBuffer, vertexCount, indexBuffer, indexCount);
}

//obj ctor
Mesh::Mesh(const char* name, const std::wstring& inputFile, bool isFBX) : name(name)
{
	


	if (isFBX)
	{
		LoadFBX(inputFile);
	}
	else
	{
		// Existing OBJ loading code
		// Author: Chris Cascioli
		// Purpose: Basic .OBJ 3D model loading, supporting positions, uvs and normals
		// 
		// - You are allowed to directly copy/paste this into your code base
		//   for assignments, given that you clearly cite that this is not
		//   code of your own design.


		// File input object
		this->m_indicesCount = 0;
		this->m_vertexCount = 0;
		std::ifstream obj(inputFile);

		// Check for successful open
		if (!obj.is_open())
			return;

		// Variables used while reading the file
		std::vector<XMFLOAT3> positions;	// Positions from the file
		std::vector<XMFLOAT3> normals;		// Normals from the file
		std::vector<XMFLOAT2> uvs;		// UVs from the file
		std::vector<Vertex> verts;		// Verts we're assembling
		std::vector<UINT> indices;		// Indices of these verts
		int vertCounter = 0;			// Count of vertices
		int indexCounter = 0;			// Count of indices
		char chars[100];			// String for line reading

		// Still have data left?
		while (obj.good())
		{
			// Get the line (100 characters should be more than enough)
			obj.getline(chars, 100);

			// Check the type of line
			if (chars[0] == 'v' && chars[1] == 'n')
			{
				// Read the 3 numbers directly into an XMFLOAT3
				XMFLOAT3 norm;
				sscanf_s(
					chars,
					"vn %f %f %f",
					&norm.x, &norm.y, &norm.z);

				// Add to the list of normals
				normals.push_back(norm);
			}
			else if (chars[0] == 'v' && chars[1] == 't')
			{
				// Read the 2 numbers directly into an XMFLOAT2
				XMFLOAT2 uv;
				sscanf_s(
					chars,
					"vt %f %f",
					&uv.x, &uv.y);

				// Add to the list of uv's
				uvs.push_back(uv);
			}
			else if (chars[0] == 'v')
			{
				// Read the 3 numbers directly into an XMFLOAT3
				XMFLOAT3 pos;
				sscanf_s(
					chars,
					"v %f %f %f",
					&pos.x, &pos.y, &pos.z);

				// Add to the positions
				positions.push_back(pos);
			}
			else if (chars[0] == 'f')
			{
				// Read the face indices into an array
				// NOTE: This assumes the given obj file contains
				//  vertex positions, uv coordinates AND normals.
				unsigned int i[12];
				int numbersRead = sscanf_s(
					chars,
					"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					&i[0], &i[1], &i[2],
					&i[3], &i[4], &i[5],
					&i[6], &i[7], &i[8],
					&i[9], &i[10], &i[11]);

				// If we only got the first number, chances are the OBJ
				// file has no UV coordinates.  This isn't great, but we
				// still want to load the model without crashing, so we
				// need to re-read a different pattern (in which we assume
				// there are no UVs denoted for any of the vertices)
				if (numbersRead == 1)
				{
					// Re-read with a different pattern
					numbersRead = sscanf_s(
						chars,
						"f %d//%d %d//%d %d//%d %d//%d",
						&i[0], &i[2],
						&i[3], &i[5],
						&i[6], &i[8],
						&i[9], &i[11]);

					// The following indices are where the UVs should 
					// have been, so give them a valid value
					i[1] = 1;
					i[4] = 1;
					i[7] = 1;
					i[10] = 1;

					// If we have no UVs, create a single UV coordinate
					// that will be used for all vertices
					if (uvs.size() == 0)
						uvs.push_back(XMFLOAT2(0, 0));
				}

				// - Create the verts by looking up
				//    corresponding data from vectors
				// - OBJ File indices are 1-based, so
				//    they need to be adusted
				Vertex v1;
				v1.Position = positions[i[0] - 1];
				v1.UV = uvs[i[1] - 1];
				v1.Normal = normals[i[2] - 1];

				Vertex v2;
				v2.Position = positions[i[3] - 1];
				v2.UV = uvs[i[4] - 1];
				v2.Normal = normals[i[5] - 1];

				Vertex v3;
				v3.Position = positions[i[6] - 1];
				v3.UV = uvs[i[7] - 1];
				v3.Normal = normals[i[8] - 1];

				// The model is most likely in a right-handed space,
				// especially if it came from Maya.  We want to convert
				// to a left-handed space for DirectX.  This means we 
				// need to:
				//  - Invert the Z position
				//  - Invert the normal's Z
				//  - Flip the winding order
				// We also need to flip the UV coordinate since DirectX
				// defines (0,0) as the top left of the texture, and many
				// 3D modeling packages use the bottom left as (0,0)

				// Flip the UV's since they're probably "upside down"
				v1.UV.y = 1.0f - v1.UV.y;
				v2.UV.y = 1.0f - v2.UV.y;
				v3.UV.y = 1.0f - v3.UV.y;

				// Flip Z (LH vs. RH)
				v1.Position.z *= -1.0f;
				v2.Position.z *= -1.0f;
				v3.Position.z *= -1.0f;

				// Flip normal's Z
				v1.Normal.z *= -1.0f;
				v2.Normal.z *= -1.0f;
				v3.Normal.z *= -1.0f;

				// Add the verts to the vector (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v3);
				verts.push_back(v2);
				vertCounter += 3;

				// Add three more indices
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;

				// Was there a 4th face?
				// - 12 numbers read means 4 faces WITH uv's
				// - 8 numbers read means 4 faces WITHOUT uv's
				if (numbersRead == 12 || numbersRead == 8)
				{
					// Make the last vertex
					Vertex v4;
					v4.Position = positions[i[9] - 1];
					v4.UV = uvs[i[10] - 1];
					v4.Normal = normals[i[11] - 1];

					// Flip the UV, Z pos and normal's Z
					v4.UV.y = 1.0f - v4.UV.y;
					v4.Position.z *= -1.0f;
					v4.Normal.z *= -1.0f;

					// Add a whole triangle (flipping the winding order)
					verts.push_back(v1);
					verts.push_back(v4);
					verts.push_back(v3);
					vertCounter += 3;

					// Add three more indices
					indices.push_back(indexCounter); indexCounter += 1;
					indices.push_back(indexCounter); indexCounter += 1;
					indices.push_back(indexCounter); indexCounter += 1;
				}
			}
		}

		// Close the file and create the actual buffers
		obj.close();
		initBuffers(&verts[0], vertCounter, &indices[0], indexCounter);

		// - At this point, "verts" is a vector of Vertex structs, and can be used
		//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
		//
		// - The vector "indices" is similar. It's a vector of unsigned ints and
		//    can be used directly for the index buffer: &indices[0] is the address of the first int
		//
		// - "vertCounter" is the number of vertices
		// - "indexCounter" is the number of indices
		// - Yes, these are effectively the same since OBJs do not index entire vertices!  This means
		//    an index buffer isn't doing much for us.  We could try to optimize the mesh ourselves
		//    and detect duplicate vertices, but at that point it would be better to use a more
		//    sophisticated model loading library like TinyOBJLoader or The Open Asset Importer Library
	}
}


void Mesh::LoadFBX(const std::wstring& filePath)
{
	scene = importer.ReadFile(std::string(filePath.begin(), filePath.end()), aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_MakeLeftHanded);


	if (!scene)
	{
		// Handle error
		return;
	}

	

	// Process the scene to extract mesh and animation data
	// This is a simplified example, you need to handle bones, weights, and animations properly

	
	// positions
	int x = 0, y = 0, z = 0;
	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;	// Positions from the file
	std::vector<XMFLOAT3> normals;		// Normals from the file
	std::vector<XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;		// Indices of these verts
	std::vector<VertexBoneData> Bones; 
	count = 0;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];

		

		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				indices.push_back(face.mIndices[k]);
			}
		}

		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex;
			vertex.Position = XMFLOAT3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			vertex.UV = XMFLOAT2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
			vertex.Normal = XMFLOAT3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			// Initialize bone weights and indices to zero
			//vertex.BoneWeights = XMFLOAT4(0, 0, 0, 0);
			//vertex.BoneIndices = XMUINT4(0, 0, 0, 0);
			verts.push_back(vertex);
			count++;
		}


		// Load bone data
		LoadBones(mesh, &Bones /*, &meshBoneData*/);

		aiMatrix4x4 offset = scene->mRootNode->mTransformation;
		meshBoneData->GlobalInverseTransform = DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
			offset.b1, offset.b2, offset.b3, offset.b4,
			offset.c1, offset.c2, offset.c3, offset.c4,
			offset.d1, offset.d2, offset.d3, offset.d4);

		meshBoneData->GlobalInverseTransform = DirectX::XMMatrixInverse(nullptr, meshBoneData->GlobalInverseTransform);

		for (unsigned int k = 0; k < Bones.size(); k++)
		{
			verts.at(k).BoneIDs.x = Bones[k].IDs[0];
			verts.at(k).BoneIDs.y = Bones[k].IDs[1];
			verts.at(k).BoneIDs.z = Bones[k].IDs[2];
			verts.at(k).BoneIDs.w = Bones[k].IDs[3];

			verts.at(k).Weights.x = Bones[k].Weights[0];
			verts.at(k).Weights.y = Bones[k].Weights[1];
			verts.at(k).Weights.z = Bones[k].Weights[2];
			verts.at(k).Weights.w = Bones[k].Weights[3];
		}

		DirectX::XMStoreFloat4x4(&meshEntity.worldMatrix, DirectX::XMMatrixIdentity());
		if (x == 80)
		{
			x = 0;
			z += 2;
		}
		meshEntity.position = DirectX::XMFLOAT3(x, y, z);
		x += 2;
		meshEntity.rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		meshEntity.scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	}


	initBuffers(verts.data(), verts.size(), indices.data(), indices.size());


}


//dtor
Mesh::~Mesh(){}

void Mesh::Draw() {
	//set buffers in the input assembler stage once per object
	//like webgpu buffers are set per frame before drawIndexed
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	//draw
	Graphics::Context->DrawIndexed(this->m_indicesCount, 0, 0);
}

void Mesh::initBuffers(Vertex* vertices, size_t numVerts, unsigned int* indices, size_t numIndices)
{
	//interleave buffer (x,y,z,color) // vertex buffer this might move later to draw once vertices move due to animation /movement
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //bind as vertex buffer
	vbd.ByteWidth = sizeof(Vertex) * (UINT)numVerts;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices;
	Graphics::Device->CreateBuffer(&vbd, &initialVertexData, m_vertexBuffer.GetAddressOf());
	this->m_vertexCount = (UINT)numVerts;

	//index buffer so we dont have to store duplicate vertices
	//since each mesh could have unique structures are not the same shape (like a square) we have to declare a new index buffer for each mesh
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; //bind as index buffer
	ibd.ByteWidth = sizeof(unsigned int) * (UINT)numIndices;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;
	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, m_indexBuffer.GetAddressOf());
	this->m_indicesCount = (UINT)numIndices;
}

void VertexBoneData::AddBoneData(UINT BoneID, float Weight)
{

	for (UINT i = 0; i < sizeof(IDs); i++)
	{
		if (Weights[i] == 0.0)
		{
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

//void Mesh::ExtractBoneWeightsForVertices(aiMesh* mesh)
//{
//	for (unsigned int i = 0; i < mesh->mNumBones; i++)
//	{
//		aiBone* bone = mesh->mBones[i];
//		std::string boneName = bone->mName.C_Str();
//
//		unsigned int boneIndex = 0;
//		if (mBoneMapping.find(boneName) == mBoneMapping.end())
//		{
//			boneIndex = mNumBones;
//			mNumBones++;
//			BoneInfo boneInfo;
//			mBoneInfo.push_back(boneInfo);
//		}
//		else
//		{
//			boneIndex = mBoneMapping[boneName];
//		}
//
//		// Store bone offset matrix
//		aiMatrix4x4 offsetMatrix = bone->mOffsetMatrix;
//		mBoneInfo[boneIndex].BoneOffset = DirectX::XMMATRIX(
//			offsetMatrix.a1, offsetMatrix.a2, offsetMatrix.a3, offsetMatrix.a4,
//			offsetMatrix.b1, offsetMatrix.b2, offsetMatrix.b3, offsetMatrix.b4,
//			offsetMatrix.c1, offsetMatrix.c2, offsetMatrix.c3, offsetMatrix.c4,
//			offsetMatrix.d1, offsetMatrix.d2, offsetMatrix.d3, offsetMatrix.d4
//		);
//
//		// Store vertex weights
//		for (unsigned int j = 0; j < bone->mNumWeights; j++)
//		{
//			unsigned int vertexID = bone->mWeights[j].mVertexId;
//			float weight = bone->mWeights[j].mWeight;
//
//			// Assign to the vertex
//			// Assuming a max of 4 bone influences per vertex
//			for (int k = 0; k < 4; k++)
//			{
//				if (mesh->mVertices[vertexID].BoneWeights[k] == 0.0f)
//				{
//					mVertices[vertexID].BoneIndices[k] = boneIndex;
//					mVertices[vertexID].BoneWeights[k] = weight;
//					break;
//				}
//			}
//		}
//	}

// ----------------------------------------------------------------------------------------------------
// - mBoneMapping - match BoneName from instantiated pScene
// - mBoneInfo - Store aiMatrx into XMMATRIX
// - Store ID and Weight for each Bone and later attach to components of the system
// - Number of Bones attached to each vertex may vary with each mesh but for now it is 1 single mesh 
// ----------------------------------------------------------------------------------------------------
void Mesh::LoadBones(aiMesh* mesh, std::vector<VertexBoneData>* BonesData/*, MeshBoneData* meshBoneData*/)
{

	BonesData->resize(mesh->mNumVertices);
	for (int i = 0; i < mesh->mNumBones; i++)
	{
		int BoneIndex = 0;
		std::string BoneName(mesh->mBones[i]->mName.data);

		if (meshBoneData->mBoneMapping.find(BoneName) == meshBoneData->mBoneMapping.end())
		{
			BoneIndex = meshBoneData->mNumBones;
			meshBoneData->mNumBones++;
			BoneInfo bi;
			meshBoneData->mBoneInfo.push_back(bi);
		}
		else
		{
			BoneIndex = meshBoneData->mBoneMapping[BoneName];
		}
		aiMatrix4x4 offset = mesh->mBones[i]->mOffsetMatrix;

		DirectX::XMMATRIX meshToBoneTransform = DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
			offset.b1, offset.b2, offset.b3, offset.b4,
			offset.c1, offset.c2, offset.c3, offset.c4,
			offset.d1, offset.d2, offset.d3, offset.d4);
		meshBoneData->mBoneInfo[BoneIndex].BoneOffset = meshToBoneTransform;
		meshBoneData->mBoneMapping[BoneName] = BoneIndex;

		for (int x = 0; x < mesh->mBones[i]->mNumWeights; x++)
		{
			int VertexID = mesh->mBones[i]->mWeights[x].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[x].mWeight;

			BonesData->at(VertexID).AddBoneData(BoneIndex, Weight);
		}
	}
}


// --------------------------------------------------------------------------------------------
// Called for each mesh having mBoneMapping[index] but for now it is set to a single Transform 
// passed to a single shader and could be improved by adding more shaders as part of ECS.
// --------------------------------------------------------------------------------------------
DirectX::XMMATRIX Mesh::BoneTransform(float TimeInSeconds, std::vector<DirectX::XMFLOAT4X4>& Transforms/*, MeshBoneData* meshBoneData*/)
{
	DirectX::XMMATRIX Identity = DirectX::XMMatrixIdentity();

	float TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ?
		scene->mAnimations[0]->mTicksPerSecond : 30.0f;
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, scene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, scene->mRootNode, Identity/*, meshBoneData*/);

	Transforms.resize(meshBoneData->mNumBones);

	for (UINT i = 0; i < meshBoneData->mNumBones; i++) {

		DirectX::XMStoreFloat4x4(&Transforms[i], meshBoneData->mBoneInfo[i].FinalTransformation);
	}

	return Identity;
}

// ---------------------------------------------------------------------------------------------------------------
// - Called recursively till all the mBoneMappin[index].FinalTransform components are calculated
// - All The calculation upto this point is based on Assimp matrices coordination and converstion of S * R * T
//   needs Transpose to match DirectX Left Hand Coordinate system
// ---------------------------------------------------------------------------------------------------------------
void Mesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform/*, MeshBoneData* meshBoneData*/)
{
	std::string NodeName(pNode->mName.data);
	aiAnimation* pAnim = scene->mAnimations[0];
	DirectX::XMMATRIX NodeTransformation = DirectX::XMMATRIX(&pNode->mTransformation.a1);
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnim, NodeName);

	if (pNodeAnim)
	{
		aiVector3D s;
		CalcInterpolatedScaling(s, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX ScalingM = DirectX::XMMatrixScaling(s.x, s.y, s.z);

		aiQuaternion q;
		CalcInterpolatedRotation(q, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX RotationM = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(q.x, q.y, q.z, q.w));

		aiVector3D t;
		CalcInterpolatedPosition(t, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX TranslationM = DirectX::XMMatrixTranslation(t.x, t.y, t.z);

		// S*R*T for DX11
		NodeTransformation = ScalingM * RotationM * TranslationM;
		NodeTransformation = XMMatrixTranspose(NodeTransformation);
	}

	DirectX::XMMATRIX GlobalTransformation = ParentTransform * NodeTransformation;

	if (meshBoneData->mBoneMapping.find(NodeName) != meshBoneData->mBoneMapping.end()) {
		UINT BoneIndex = meshBoneData->mBoneMapping[NodeName];
		meshBoneData->mBoneInfo[BoneIndex].FinalTransformation = meshBoneData->GlobalInverseTransform * GlobalTransformation * meshBoneData->mBoneInfo[BoneIndex].BoneOffset;
	}

	for (UINT i = 0; i < pNode->mNumChildren; ++i) {

		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation/*, meshBoneData*/);
	}

}

const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (UINT i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void Mesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	UINT PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	UINT NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void Mesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	UINT ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	UINT NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void Mesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	UINT RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	UINT NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

UINT Mesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


UINT Mesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

UINT Mesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}
