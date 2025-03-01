#include "Material.h"
#include <iostream>
//static map definition when u declare it in the header file it needs to be defined in the cpp file as well
std::unordered_map<std::shared_ptr<ISimpleShader>, std::vector<std::shared_ptr<Material>>> Material::sharedVertexShaders;

//ctor
Material::Material(const char* name, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<ISimpleShader> vs, DirectX::XMFLOAT3 tint, float rough) :
    name(name),
    pixelShader(ps),
    vertexShader(vs),
    colorTint(tint),
    roughness(rough)
{

}

//#region unimportant


//dtor
Material::~Material()
{
}

//getters and setters
const char* Material::GetName()
{
    return name;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
    return pixelShader;
}

std::shared_ptr<ISimpleShader> Material::GetVertexShader()
{
    return vertexShader;
}

DirectX::XMFLOAT3 Material::GetColorTint() const
{
    return colorTint;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> ps)
{
    pixelShader = ps;
}

void Material::SetVertexShader(std::shared_ptr<ISimpleShader> vs)
{
    vertexShader = vs;
}

void Material::SetColorTint(DirectX::XMFLOAT3 color)
{
    colorTint = color;
}

void Material::SetRoughness(float rough)
{
    roughness = rough;
}



void Material::Initialize()
{
    //adds material to sharedVertexShader map to sort for a perFrameData optimization
    RegisterMaterialWithShader();
}

void Material::RegisterMaterialWithShader()
{
    auto it = sharedVertexShaders.find(vertexShader);
    if (it == sharedVertexShaders.end())
    {
        sharedVertexShaders[vertexShader] = std::vector<std::shared_ptr<Material>>();
    }
    // shared_from_this means "give me a shared_ptr to this object" 
	// (as opposed to a weak_ptr, and only works if the object is already managed by a shared_ptr)
	// so the reason we need to use shared_from_this is because the shared ptrs are not initialized in the ctor
    //so essentially, we are adding this material to the vector of materials that use this shader
    sharedVertexShaders[vertexShader].push_back(shared_from_this());
    std::string shaderName = vertexShader->GetShaderName(); // Assuming GetName() exists
    std::cout << "Shader: " << shaderName
        << ", Number of materials using this shader: "
        << sharedVertexShaders[vertexShader].size() << std::endl;
}




//LIKELY THIS AND SIMPLE SHADER NEEDS A REWRITE
void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
    // 'Turn on' these shaders
    vertexShader->SetShader();
    pixelShader->SetShader();

    vertexShader->SetMatrix4x4("world", transform->getWorldMatrix());
    vertexShader->SetMatrix4x4("worldInvTrans", transform->getWorldInverseTransposeMatrix());

    // View and projection matrices are likely per-frame and always need to be set
    vertexShader->SetMatrix4x4("view", camera->getViewMatrix());
    vertexShader->SetMatrix4x4("projection", camera->getProjectionMatrix());
    vertexShader->CopyAllBufferData();

    // Pixel shader settings
    pixelShader->SetFloat3("colorTint", colorTint);
    pixelShader->CopyAllBufferData();
}

