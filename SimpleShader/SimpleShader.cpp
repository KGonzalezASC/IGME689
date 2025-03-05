#include "SimpleShader.h"

#include <filesystem>
#include <iostream>

// Default error reporting state
bool ISimpleShader::ReportErrors = false;
bool ISimpleShader::ReportWarnings = false;

// To enable error reporting, use either or both 
// of the following lines somewhere in your program, 
// preferably before loading/using any shaders.
// 
// ISimpleShader::ReportErrors = true;
// ISimpleShader::ReportWarnings = true;


///////////////////////////////////////////////////////////////////////////////
// ------ BASE SIMPLE SHADER --------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------
// Constructor accepts Direct3D device & context
// --------------------------------------------------------
ISimpleShader::ISimpleShader(Microsoft::WRL::ComPtr<ID3D11Device1> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context)
{
	// Save the device
	this->device = device;
	this->deviceContext = context;

	// Set up fields
	this->constantBufferCount = 0;
	this->constantBuffers = 0;
	this->shaderValid = false;
}

ISimpleShader::~ISimpleShader()
{
	// Derived class destructors will call this class's CleanUp method
}

void ISimpleShader::CleanUp()
{
	// Handle constant buffers and local data buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		delete[] constantBuffers[i].LocalDataBuffer;
	}

	if (constantBuffers)
	{
		delete[] constantBuffers;
		constantBufferCount = 0;
	}

	for (unsigned int i = 0; i < shaderResourceViews.size(); i++)
		delete shaderResourceViews[i];

	for (unsigned int i = 0; i < samplerStates.size(); i++)
		delete samplerStates[i];

	// Clean up tables
	varTable.clear();
	cbTable.clear();
	samplerTable.clear();
	textureTable.clear();
}

bool ISimpleShader::LoadShaderFile(LPCWSTR shaderFile)
{
	// Load the shader to a blob and ensure it worked
	HRESULT hr = D3DReadFileToBlob(shaderFile, shaderBlob.GetAddressOf());
	if (hr != S_OK)
	{
		if (ReportErrors)
		{
			LogError("SimpleShader::LoadShaderFile() - Error loading file '");
			LogW(shaderFile);
			LogError("'. Ensure this file exists and is spelled correctly.\n");
		}

		return false;
	}

	// Create the shader - Calls an overloaded version of this abstract
	// method in the appropriate child class
	shaderValid = CreateShader(shaderBlob);
	if (!shaderValid)
	{
		if (ReportErrors)
		{
			LogError("SimpleShader::LoadShaderFile() - Error creating shader from file '");
			LogW(shaderFile);
			LogError("'. Ensure the type of shader (vertex, pixel, etc.) matches the SimpleShader type (SimpleVertexShader, SimplePixelShader, etc.) you're using.\n");
		}

		return false;
	}
	std::wstring fileName = std::filesystem::path(shaderFile).stem();
	shaderName = std::string(fileName.begin(), fileName.end());

	// Set up shader reflection to get information about
	// this shader and its variables,  buffers, etc.
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)refl.GetAddressOf());

	// Get the description of the shader
	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);

	// Create resource arrays
	constantBufferCount = shaderDesc.ConstantBuffers;
	constantBuffers = new SimpleConstantBuffer[constantBufferCount];

	// Handle bound resources (like shaders and samplers)
	unsigned int resourceCount = shaderDesc.BoundResources;
	for (unsigned int r = 0; r < resourceCount; r++)
	{
		// Get this resource's description
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		refl->GetResourceBindingDesc(r, &resourceDesc);

		// Check the type
		switch (resourceDesc.Type)
		{
		case D3D_SIT_STRUCTURED: // Treat structured buffers as texture resources
		case D3D_SIT_TEXTURE: // A texture resource
		{
			// Create the SRV wrapper
			SimpleSRV* srv = new SimpleSRV();
			srv->BindIndex = resourceDesc.BindPoint;				// Shader bind point
			srv->Index = (unsigned int)shaderResourceViews.size();	// Raw index

			textureTable.insert(std::pair<std::string, SimpleSRV*>(resourceDesc.Name, srv));
			shaderResourceViews.push_back(srv);
		}
		break;

		case D3D_SIT_SAMPLER: // A sampler resource
		{
			// Create the sampler wrapper
			SimpleSampler* samp = new SimpleSampler();
			samp->BindIndex = resourceDesc.BindPoint;			// Shader bind point
			samp->Index = (unsigned int)samplerStates.size();	// Raw index

			samplerTable.insert(std::pair<std::string, SimpleSampler*>(resourceDesc.Name, samp));
			samplerStates.push_back(samp);
		}
		break;
		}
	}

	// Loop through all constant buffers
	for (unsigned int b = 0; b < constantBufferCount; b++)
	{
		// Get this buffer
		ID3D11ShaderReflectionConstantBuffer* cb =
			refl->GetConstantBufferByIndex(b);

		// Get the description of this buffer
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		cb->GetDesc(&bufferDesc);

		// Save the type, which we reference when setting these buffers
		constantBuffers[b].Type = bufferDesc.Type;

		// Get the description of the resource binding, so
		// we know exactly how it's bound in the shader
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		refl->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

		// Set up the buffer and put its pointer in the table
		constantBuffers[b].BindIndex = bindDesc.BindPoint;
		constantBuffers[b].Name = bufferDesc.Name;
		cbTable.insert(std::pair<std::string, SimpleConstantBuffer*>(bufferDesc.Name, &constantBuffers[b]));

		// Create this constant buffer
		D3D11_BUFFER_DESC newBuffDesc = {};
		newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
		newBuffDesc.ByteWidth = ((bufferDesc.Size + 15) / 16) * 16; // Quick and dirty 16-byte alignment using integer division
		newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		newBuffDesc.CPUAccessFlags = 0;
		newBuffDesc.MiscFlags = 0;
		newBuffDesc.StructureByteStride = 0;
		device->CreateBuffer(&newBuffDesc, 0, constantBuffers[b].ConstantBuffer.GetAddressOf());

		// Set up the data buffer for this constant buffer
		constantBuffers[b].Size = bufferDesc.Size;
		constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
		ZeroMemory(constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

		// Loop through all variables in this buffer
		for (unsigned int v = 0; v < bufferDesc.Variables; v++)
		{
			// Get this variable
			ID3D11ShaderReflectionVariable* var =
				cb->GetVariableByIndex(v);

			// Get the description of the variable and its type
			D3D11_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			// Create the variable struct
			SimpleShaderVariable varStruct = {};
			varStruct.ConstantBufferIndex = b;
			varStruct.ByteOffset = varDesc.StartOffset;
			varStruct.Size = varDesc.Size;

			// Get a string version
			std::string varName(varDesc.Name);

			// Add this variable to the table and the constant buffer
			varTable.insert(std::pair<std::string, SimpleShaderVariable>(varName, varStruct));
			constantBuffers[b].Variables.push_back(varStruct);
		}
	}

	// All set
	return true;
}

// --------------------------------------------------------
// Helper for looking up a variable by name and also
// verifying that it is the requested size
// 
// name - the name of the variable to look for
// size - the size of the variable (for verification), or -1 to bypass
// --------------------------------------------------------
SimpleShaderVariable* ISimpleShader::FindVariable(std::string name, int size)
{
	// Look for the key
	std::unordered_map<std::string, SimpleShaderVariable>::iterator result =
		varTable.find(name);

	// Did we find the key?
	if (result == varTable.end())
		return 0;

	// Grab the result from the iterator
	SimpleShaderVariable* var = &(result->second);

	// Is the data size correct ?
	if (size > 0 && var->Size != size)
		return 0;

	// Success
	return var;
}
SimpleConstantBuffer* ISimpleShader::FindConstantBuffer(std::string name)
{
	// Look for the key
	std::unordered_map<std::string, SimpleConstantBuffer*>::iterator result =
		cbTable.find(name);

	// Did we find the key?
	if (result == cbTable.end())
		return 0;

	// Success
	return result->second;
}
void ISimpleShader::Log(std::string message, WORD color)
{
	// Swap console color
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);

	printf_s(message.c_str());
	OutputDebugStringA(message.c_str());

	// Swap back
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
void ISimpleShader::LogW(std::wstring message, WORD color)
{
	// Swap console color
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);

	wprintf_s(message.c_str());
	OutputDebugStringW(message.c_str());

	// Swap back
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// Helpers for pritning errors and warnings in specific colors using regular and wide character strings
void ISimpleShader::Log(std::string message) { Log(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
void ISimpleShader::LogW(std::wstring message) { LogW(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); }
void ISimpleShader::LogError(std::string message) { Log(message, FOREGROUND_RED | FOREGROUND_INTENSITY); }
void ISimpleShader::LogErrorW(std::wstring message) { LogW(message, FOREGROUND_RED | FOREGROUND_INTENSITY); }
void ISimpleShader::LogWarning(std::string message) { Log(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
void ISimpleShader::LogWarningW(std::wstring message) { LogW(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); }
void ISimpleShader::SetShader()
{
	// Ensure the shader is valid
	if (!shaderValid) return;
	// Set the shader and any relevant constant buffers, which
	// is an overloaded method in a subclass
	SetShaderAndCBs();
}

void ISimpleShader::CopyAllBufferData()
{
	// Ensure the shader is valid
	if (!shaderValid) return;

	// Loop through the constant buffers and copy all data
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Copy the entire local data buffer
		deviceContext->UpdateSubresource(
			constantBuffers[i].ConstantBuffer.Get(), 0, 0,
			constantBuffers[i].LocalDataBuffer, 0, 0);
	}
}

// --------------------------------------------------------
// Copies local data to the shader's specified constant buffer
//
// index - The index of the buffer to copy.
//         Useful for updating more frequently-changing
//         variables without having to re-copy all buffers.
//  
// NOTE: The "index" of the buffer might NOT be the same
//       as its register, especially if you have buffers
//       bound to non-sequential registers!
// --------------------------------------------------------
void ISimpleShader::CopyBufferData(unsigned int index)
{
	// Ensure the shader is valid
	if (!shaderValid) return;

	// Validate the index
	if (index >= this->constantBufferCount)
		return;

	// Check for the buffer
	SimpleConstantBuffer* cb = &this->constantBuffers[index];
	if (!cb) return;

	// Copy the data and get out
	deviceContext->UpdateSubresource(
		cb->ConstantBuffer.Get(), 0, 0,
		cb->LocalDataBuffer, 0, 0);
}

// --------------------------------------------------------
// Copies local data to the shader's specified constant buffer
void ISimpleShader::CopyBufferData(std::string bufferName)
{
	// Ensure the shader is valid
	if (!shaderValid) return;

	// Check for the buffer
	SimpleConstantBuffer* cb = this->FindConstantBuffer(bufferName);
	if (!cb) return;

	// Copy the data and get out
	deviceContext->UpdateSubresource(
		cb->ConstantBuffer.Get(), 0, 0,
		cb->LocalDataBuffer, 0, 0);
}


// --------------------------------------------------------
// Sets a variable by name with arbitrary data of the specified size
//
// name - The name of the shader variable
// data - The data to set in the buffer
// size - The size of the data (this must be less than or equal to the variable's size)
//
// Returns true if data is copied, false if variable doesn't exist
// --------------------------------------------------------
bool ISimpleShader::SetData(std::string name, const void* data, unsigned int size)
{
	// Look for the variable and verify
	SimpleShaderVariable* var = FindVariable(name, -1);
	if (var == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("SimpleShader::SetData() - Shader variable '");
			Log(name);
			LogWarning("' not found. Ensure the name is spelled correctly and that it exists in a constant buffer in the shader.\n");
		}
		return false;
	}
	// Ensure we're not trying to copy more data than the variable can hold
	// Note: We can copy less data, in the case of a subset of an array
	if (size > var->Size)
	{
		if (ReportWarnings)
		{
			LogWarning("SimpleShader::SetData() - Shader variable '");
			Log(name);
			LogWarning("' is smaller than the size of the data being set. Ensure the variable is large enough for the specified data.\n");
		}
		return false;
	}

	// Set the data in the local data buffer
	memcpy(
		constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
		data,
		size);

	// Success
	return true;
}

bool ISimpleShader::SetInt(std::string name, int data)
{
	return this->SetData(name, (void*)(&data), sizeof(int));
}

bool ISimpleShader::SetFloat(std::string name, float data)
{
	return this->SetData(name, (void*)(&data), sizeof(float));
}

bool ISimpleShader::SetFloat2(std::string name, const float data[2])
{
	return this->SetData(name, (void*)data, sizeof(float) * 2);
}

bool ISimpleShader::SetFloat2(std::string name, const DirectX::XMFLOAT2 data)
{
	return this->SetData(name, &data, sizeof(float) * 2);
}

bool ISimpleShader::SetFloat3(std::string name, const float data[3])
{
	return this->SetData(name, (void*)data, sizeof(float) * 3);
}

bool ISimpleShader::SetFloat3(std::string name, const DirectX::XMFLOAT3 data)
{
	return this->SetData(name, &data, sizeof(float) * 3);
}

bool ISimpleShader::SetFloat4(std::string name, const float data[4])
{
	return this->SetData(name, (void*)data, sizeof(float) * 4);
}

bool ISimpleShader::SetFloat4(std::string name, const DirectX::XMFLOAT4 data)
{
	return this->SetData(name, &data, sizeof(float) * 4);
}

bool ISimpleShader::SetMatrix4x4(std::string name, const float data[16])
{
	return this->SetData(name, (void*)data, sizeof(float) * 16);
}

bool ISimpleShader::SetMatrix4x4(std::string name, const DirectX::XMFLOAT4X4 data)
{
	return this->SetData(name, &data, sizeof(float) * 16);
}

bool ISimpleShader::HasVariable(std::string name)
{
	return FindVariable(name, -1) != 0;
}

bool ISimpleShader::HasShaderResourceView(std::string name)
{
	return GetShaderResourceViewInfo(name) != 0;
}

bool ISimpleShader::HasSamplerState(std::string name)
{
	return GetSamplerInfo(name) != 0;
}

const SimpleShaderVariable* ISimpleShader::GetVariableInfo(std::string name)
{
	return FindVariable(name, -1);
}

const SimpleSRV* ISimpleShader::GetShaderResourceViewInfo(std::string name)
{
	// Look for the key
	std::unordered_map<std::string, SimpleSRV*>::iterator result =
		textureTable.find(name);

	// Did we find the key?
	if (result == textureTable.end())
		return 0;

	// Success
	return result->second;
}

const SimpleSRV* ISimpleShader::GetShaderResourceViewInfo(unsigned int index)
{
	// Valid index?
	if (index >= shaderResourceViews.size()) return 0;

	// Grab the bind index
	return shaderResourceViews[index];
}

const SimpleSampler* ISimpleShader::GetSamplerInfo(std::string name)
{
	// Look for the key
	std::unordered_map<std::string, SimpleSampler*>::iterator result =
		samplerTable.find(name);

	// Did we find the key?
	if (result == samplerTable.end())
		return 0;

	// Success
	return result->second;
}

const SimpleSampler* ISimpleShader::GetSamplerInfo(unsigned int index)
{
	// Valid index?
	if (index >= samplerStates.size()) return 0;

	// Grab the bind index
	return samplerStates[index];
}

// --------------------------------------------------------
unsigned int ISimpleShader::GetBufferCount() { return constantBufferCount; }

unsigned int ISimpleShader::GetBufferSize(unsigned int index)
{
	// Valid index?
	if (index >= constantBufferCount)
		return -1;

	// Grab the size
	return constantBuffers[index].Size;
}

const SimpleConstantBuffer* ISimpleShader::GetBufferInfo(std::string name)
{
	return FindConstantBuffer(name);
}

SimpleConstantBuffer* ISimpleShader::GetBufferInfo(unsigned int index)
{
	// Check for valid index
	if (index >= constantBufferCount) return 0;

	// Return the specific buffer
	return &constantBuffers[index];
}

///////////////////////////////////////////////////////////////////////////////
// ------ SIMPLE VERTEX SHADER ------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------
// Constructor just calls the base
// --------------------------------------------------------
SimpleVertexShader::SimpleVertexShader(Microsoft::WRL::ComPtr<ID3D11Device1> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context, LPCWSTR shaderFile)
	: ISimpleShader(device, context)
{
	// Ensure we set to zero to successfully trigger
	// the Input Layout creation during LoadShaderFile()
	this->perInstanceCompatible = false;

	// Load the actual compiled shader file
	this->LoadShaderFile(shaderFile);
}

// --------------------------------------------------------
// Constructor overload which takes a custom input layout
//
// Passing in a valid input layout will stop LoadShaderFile()
// from creating an input layout from shader reflection
// --------------------------------------------------------
SimpleVertexShader::SimpleVertexShader(Microsoft::WRL::ComPtr<ID3D11Device1> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context, LPCWSTR shaderFile,
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout,
	bool perInstanceCompatible)
	: ISimpleShader(device, context)
{
	// Save the custom input layout
	this->inputLayout = inputLayout;

	// Unable to determine from an input layout, require user to tell us
	this->perInstanceCompatible = perInstanceCompatible;

	// Load the actual compiled shader file
	this->LoadShaderFile(shaderFile);
}

// --------------------------------------------------------
// Destructor - Clean up actual shader (base will be called automatically)
// --------------------------------------------------------
SimpleVertexShader::~SimpleVertexShader()
{
	CleanUp();
}

// --------------------------------------------------------
// Handles cleaning up shader and base class clean up
// --------------------------------------------------------
void SimpleVertexShader::CleanUp()
{
	ISimpleShader::CleanUp();
}

// --------------------------------------------------------
// Creates the  Direct3D vertex shader
//
// shaderBlob - The shader's compiled code
//
// Returns true if shader is created correctly, false otherwise
// --------------------------------------------------------
bool SimpleVertexShader::CreateShader(Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob)
{
	// Clean up first, in the event this method is
	// called more than once on the same object
	this->CleanUp();

	// Create the shader from the blob
	HRESULT result = device->CreateVertexShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		0,
		shader.GetAddressOf());

	// Did the creation work?
	if (result != S_OK)
		return false;

	// Do we already have an input layout?
	// (This would come from one of the constructor overloads)
	if (inputLayout)
		return true;

	// Vertex shader was created successfully, so we now use the
	// shader code to re-reflect and create an input layout that 
	// matches what the vertex shader expects.  Code adapted from:
	// https://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/

	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)refl.GetAddressOf());

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		refl->GetInputParameterDesc(i, &paramDesc);

		// Check the semantic name for "_PER_INSTANCE"
		std::string perInstanceStr = "_PER_INSTANCE";
		std::string sem = paramDesc.SemanticName;
		int lenDiff = (int)sem.size() - (int)perInstanceStr.size();
		bool isPerInstance =
			lenDiff >= 0 &&
			sem.compare(lenDiff, perInstanceStr.size(), perInstanceStr) == 0;

		// Fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Replace anything affected by "per instance" data
		if (isPerInstance)
		{
			elementDesc.InputSlot = 1; // Assume per instance data comes from another input slot!
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;

			perInstanceCompatible = true;
		}

		// Determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		// Save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = device->CreateInputLayout(
		&inputLayoutDesc[0],
		(unsigned int)inputLayoutDesc.size(),
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		inputLayout.GetAddressOf());

	// All done, clean up
	return true;
}

// --------------------------------------------------------
// Sets the vertex shader, input layout and constant buffers
// for future  Direct3D drawing
// --------------------------------------------------------
void SimpleVertexShader::SetShaderAndCBs()
{
	// Is shader valid?
	if (!shaderValid) return;

	// Set the shader and input layout
	deviceContext->IASetInputLayout(inputLayout.Get());
	deviceContext->VSSetShader(shader.Get(), 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Skip "buffers" that aren't true constant buffers
		if (constantBuffers[i].Type != D3D11_CT_CBUFFER)
			continue;

		// This is a real constant buffer, so set it
		deviceContext->VSSetConstantBuffers(
			constantBuffers[i].BindIndex,
			1,
			constantBuffers[i].ConstantBuffer.GetAddressOf());
	}
}

bool SimpleVertexShader::SetShaderResourceView(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	// Look for the variable and verify
	const SimpleSRV* srvInfo = GetShaderResourceViewInfo(name);
	if (srvInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("SimpleVertexShader::SetShaderResourceView() - SRV named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}

	// Set the shader resource view
	deviceContext->VSSetShaderResources(srvInfo->BindIndex, 1, srv.GetAddressOf());

	// Success
	return true;
}

bool SimpleVertexShader::SetSamplerState(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	// Look for the variable and verify
	const SimpleSampler* sampInfo = GetSamplerInfo(name);
	if (sampInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("SimpleVertexShader::SetSamplerState() - Sampler named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}

	// Set the shader resource view
	deviceContext->VSSetSamplers(sampInfo->BindIndex, 1, samplerState.GetAddressOf());

	// Success
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// ------ SIMPLE PIXEL SHADER -------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
SimplePixelShader::SimplePixelShader(Microsoft::WRL::ComPtr<ID3D11Device1> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context, LPCWSTR shaderFile)
	: ISimpleShader(device, context)
{
	// Load the actual compiled shader file
	this->LoadShaderFile(shaderFile);
}

SimplePixelShader::~SimplePixelShader()
{
	CleanUp();
}

void SimplePixelShader::CleanUp()
{
	ISimpleShader::CleanUp();
}

bool SimplePixelShader::CreateShader(Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob)
{
	// Clean up first, in the event this method is
	// called more than once on the same object
	this->CleanUp();

	// Create the shader from the blob
	HRESULT result = device->CreatePixelShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		0,
		shader.GetAddressOf());

	// Check the result
	return (result == S_OK);
}

void SimplePixelShader::SetShaderAndCBs()
{
	// Is shader valid?
	if (!shaderValid) return;

	// Set the shader
	deviceContext->PSSetShader(shader.Get(), 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Skip "buffers" that aren't true constant buffers
		if (constantBuffers[i].Type != D3D11_CT_CBUFFER)
			continue;

		// This is a real constant buffer, so set it
		deviceContext->PSSetConstantBuffers(
			constantBuffers[i].BindIndex,
			1,
			constantBuffers[i].ConstantBuffer.GetAddressOf());
	}
}

bool SimplePixelShader::SetShaderResourceView(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	// Look for the variable and verify
	const SimpleSRV* srvInfo = GetShaderResourceViewInfo(name);
	if (srvInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("SimplePixelShader::SetShaderResourceView() - SRV named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}

	// Set the shader resource view
	deviceContext->PSSetShaderResources(srvInfo->BindIndex, 1, srv.GetAddressOf());

	// Success
	return true;
}

bool SimplePixelShader::SetSamplerState(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	// Look for the variable and verify
	const SimpleSampler* sampInfo = GetSamplerInfo(name);
	if (sampInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("SimplePixelShader::SetSamplerState() - Sampler named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}

	// Set the shader resource view
	deviceContext->PSSetSamplers(sampInfo->BindIndex, 1, samplerState.GetAddressOf());

	// Success
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///Less Simple Vertex Shader
///////////////////////////////////////////////////////////////////////////////
//ctor inherits from ISimpleShader
size_t LessSimpleVertexShader::GetObjectDataSize(ID3D11ShaderReflectionConstantBuffer* cb)
{
	size_t objectDataSize = 0;

	// Iterate over all variables in the constant buffer to calculate the total size of the constant buffer
	D3D11_SHADER_BUFFER_DESC bufferDesc;
	cb->GetDesc(&bufferDesc);

	for (unsigned int v = 0; v < bufferDesc.Variables; v++)
	{
		ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
		D3D11_SHADER_VARIABLE_DESC varDesc;
		var->GetDesc(&varDesc);

		// Add the size of each variable to the object data size
		objectDataSize += varDesc.Size;
	}

	return objectDataSize; // Return the total object data size
}

LessSimpleVertexShader::LessSimpleVertexShader(Microsoft::WRL::ComPtr<ID3D11Device1> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context, LPCWSTR shaderFile)
	: ISimpleShader(device, context)
{
	this->perInstanceCompatible = false;
	this->LoadShaderFile(shaderFile);
}

LessSimpleVertexShader::~LessSimpleVertexShader()
{
	CleanUp();
}
void LessSimpleVertexShader::CleanUp()
{
	ISimpleShader::CleanUp();
}

bool LessSimpleVertexShader::CreateShader(Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob)
{
	// Clean up first, in the event this method is
		// called more than once on the same object
	this->CleanUp();

	// Create the shader from the blob
	HRESULT result = device->CreateVertexShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		nullptr,
		shader.GetAddressOf());

	// Did the creation work?
	if (result != S_OK)
		return false;

	// Do we already have an input layout?
	// (This would come from one of the constructor overloads)
	if (inputLayout)
		return true;

	// Vertex shader was created successfully, so we now use the
	// shader code to re-reflect and create an input layout that 
	// matches what the vertex shader expects.  Code adapted from:
	// https://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/

	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)refl.GetAddressOf());

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		refl->GetInputParameterDesc(i, &paramDesc);

		// Check the semantic name for "_PER_INSTANCE"
		std::string perInstanceStr = "_PER_INSTANCE";
		std::string sem = paramDesc.SemanticName;
		int lenDiff = static_cast<int>(sem.size()) - static_cast<int>(perInstanceStr.size());
		bool isPerInstance =
			lenDiff >= 0 &&
			sem.compare(lenDiff, perInstanceStr.size(), perInstanceStr) == 0;

		// Fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Replace anything affected by "per instance" data
		if (isPerInstance)
		{
			elementDesc.InputSlot = 1; // Assume per instance data comes from another input slot!
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;

			perInstanceCompatible = true;
		}

		// Determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format =
				DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format =
				DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format =
				DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format =
				DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format =
				DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		// Save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	HRESULT hr = device->CreateInputLayout(
		&inputLayoutDesc[0],
		static_cast<unsigned int>(inputLayoutDesc.size()),
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		inputLayout.GetAddressOf());

	return true;
}

constexpr size_t PerObjectDataSizeAligned = 256; // YOU MUST ENFORCE 256 BYTE ALIGNMENT ON THE GPU BUFFER BECAUSE OF THE POOLING


bool LessSimpleVertexShader::LoadShaderFile(LPCWSTR shaderFile)
{
	//part that can stay the same?
	{
		HRESULT hr = D3DReadFileToBlob(shaderFile, shaderBlob.GetAddressOf());
		if (hr != S_OK)
		{
			if (ReportErrors)
			{
				LogError("SimpleShader::LoadShaderFile() - Error loading file '");
				LogW(shaderFile);
				LogError("'. Ensure this file exists and is spelled correctly.\n");
			}

			return false;
		}

		// Create the shader - Calls an overloaded version of this abstract
		// method in the appropriate child class
		shaderValid = CreateShader(shaderBlob);
		if (!shaderValid)
		{
			if (ReportErrors)
			{
				LogError("SimpleShader::LoadShaderFile() - Error creating shader from file '");
				LogW(shaderFile);
				LogError(
					"'. Ensure the type of shader (vertex, pixel, etc.) matches the SimpleShader type (SimpleVertexShader, SimplePixelShader, etc.) you're using.\n");
			}

			return false;
		}
		//print out name of shader file as in not the path or the .cso part
		std::wstring fileName = std::filesystem::path(shaderFile).stem();
		shaderName = std::string(fileName.begin(), fileName.end());
	}
	//we need specific reflection requirements, if cbuffer is called PerObjectData buffer we make dynamic buffer desc else, use one similar to the inherited method
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)refl.GetAddressOf());

	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);
	//create resource arrays
	constantBufferCount = shaderDesc.ConstantBuffers;
	constantBuffers = new SimpleConstantBuffer[constantBufferCount];
	//we do not need to handle bound resources, as vertex shaders usually do not have them if patrick gets to textureSRVS however we will need to handle them
	//Loop through all constant buffers
	for (unsigned int b = 0; b < constantBufferCount; b++)
	{
		// Retrieve this constant buffer using shader reflection
		ID3D11ShaderReflectionConstantBuffer* cb =
			refl->GetConstantBufferByIndex(b);

		// Get the buffer's description, which includes:
		// - The buffer's name (if specified in the shader)
		// - The total byte size of the buffer
		// - The type of buffer (constant buffer, structured buffer, etc.)
		// - The number of variables within the buffer
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		cb->GetDesc(&bufferDesc);

		// Store the buffer type (e.g., constant buffer), 
		constantBuffers[b].Type = bufferDesc.Type; //is almost always constant buffer

		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		refl->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

		constantBuffers[b].BindIndex = bindDesc.BindPoint;
		constantBuffers[b].Name = bufferDesc.Name;
		cbTable.insert(std::pair<std::string, SimpleConstantBuffer*>(bufferDesc.Name, &constantBuffers[b]));

		//if the name is PerObjectData, we need to create a dynamic buffer desc/ i can use semantics but that is on variable names not the cbuffer declaration
		if (strcmp(bufferDesc.Name, "PerObjectData") == 0)
		{
			std::cout << "Valid constant buffer type to join pool. " << bufferDesc.Name;
			D3D11_BUFFER_DESC newBuffDesc = {};
			newBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
			//determine size by the number of variables in cbuffer * an arbitrary count of objects
			//size_t objectDataSize = GetObjectDataSize(cb);
			size_t poolBufferSize = PerObjectDataSizeAligned * MAX_OBJECTS;
			newBuffDesc.ByteWidth = ((poolBufferSize + 15) / 16) * 16;
			// Quick and dirty 16-byte alignment using integer division
			newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			newBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			newBuffDesc.MiscFlags = 0;
			newBuffDesc.StructureByteStride = 0;
			HRESULT hrCreate = device->CreateBuffer(&newBuffDesc, nullptr,
				constantBuffers[b].ConstantBuffer.GetAddressOf());
			if (FAILED(hrCreate))
			{
				std::cout << "Failed to create pool buffer!" << std::endl;
				return false;
			}
			//indicate size on wrapper
			constantBuffers[b].Size = poolBufferSize;
			constantBuffers[b].LocalDataBuffer = new unsigned char[poolBufferSize];
			constantBuffers[b].SubBufferOffsets.resize(MAX_OBJECTS);
			ZeroMemory(constantBuffers[b].LocalDataBuffer, poolBufferSize);
			for (unsigned int v = 0; v < bufferDesc.Variables; v++)
			{
				ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
				D3D11_SHADER_VARIABLE_DESC varDesc;
				var->GetDesc(&varDesc);
				SimpleShaderVariable varStruct = {};
				varStruct.ConstantBufferIndex = b;
				varStruct.ByteOffset = varDesc.StartOffset;
				varStruct.Size = varDesc.Size;
				std::string varName(varDesc.Name);
				varTable.insert(std::pair<std::string, SimpleShaderVariable>(varName, varStruct));
				constantBuffers[b].Variables.push_back(varStruct);
			}
			constantBuffers[b].isPoolBuffer = true;
			std::cout << " Size: " << newBuffDesc.ByteWidth << std::endl;
		}
		else
		{
			std::cout << "Buffer does not need to be pooled. USAGE_DEFAULT " << bufferDesc.Name;
			D3D11_BUFFER_DESC newBuffDesc = {};
			newBuffDesc.Usage = D3D11_USAGE_DEFAULT;
			newBuffDesc.ByteWidth = ((bufferDesc.Size + 15) / 16) * 16;
			// Quick and dirty 16-byte alignment using integer division
			newBuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			newBuffDesc.CPUAccessFlags = 0;
			newBuffDesc.MiscFlags = 0;
			newBuffDesc.StructureByteStride = 0;
			device->CreateBuffer(&newBuffDesc, nullptr, constantBuffers[b].ConstantBuffer.GetAddressOf());

			// Set up the data buffer for this constant buffer
			constantBuffers[b].Size = bufferDesc.Size;
			constantBuffers[b].LocalDataBuffer = new unsigned char[bufferDesc.Size];
			ZeroMemory(constantBuffers[b].LocalDataBuffer, bufferDesc.Size);

			// Loop through all variables in this buffer
			for (unsigned int v = 0; v < bufferDesc.Variables; v++)
			{
				// Get this variable
				ID3D11ShaderReflectionVariable* var =
					cb->GetVariableByIndex(v);

				// Get the description of the variable and its type
				D3D11_SHADER_VARIABLE_DESC varDesc;
				var->GetDesc(&varDesc);

				// Create the variable struct
				SimpleShaderVariable varStruct = {};
				varStruct.ConstantBufferIndex = b;
				varStruct.ByteOffset = varDesc.StartOffset;
				varStruct.Size = varDesc.Size;

				// Get a string version
				std::string varName(varDesc.Name);

				// Add this variable to the table and the constant buffer
				varTable.insert(std::pair<std::string, SimpleShaderVariable>(varName, varStruct));
				constantBuffers[b].Variables.push_back(varStruct);
			}
			//print out buffer name and usage and size
			std::cout << " Size: " << newBuffDesc.ByteWidth << std::endl;
		}
	}
	std::cout << "Less Simple Vertex Shader is ready " << std::endl;
	return true;
}


bool LessSimpleVertexShader::WFillPerObjectDataBuffer(unsigned int objectIndex, const void* data)
{
	auto cb = GetBufferInfo(1);
	if (!cb) return false;

	size_t objectDataSize = sizeof(DirectX::XMFLOAT4X4) * 2; // Two matrices (128 bytes)
	//~ means bitwise not and ensures the offset is a multiple of the aligned size
	size_t offset = (objectIndex * PerObjectDataSizeAligned);

	if (offset + objectDataSize > cb->Size)
	{
		std::cerr << "Error: Trying to write beyond the allocated space for the buffer!" << std::endl;
		return false;
	}

	// Write both matrices to the local CPU-side buffer
	memcpy(cb->LocalDataBuffer + offset, data, objectDataSize);
	cb->SubBufferOffsets[objectIndex] = offset;

	// Debug Output
	//std::cout << "[DEBUG] Writing PerObjectData at offset: " << offset << std::endl;
	return true;
}



//copies object data to the GPU using MAP/UNMAP since updateSubresource does not let us copy to a subregion of the buffer
bool LessSimpleVertexShader::CopyPerObjectData(unsigned int objectIndex, bool isDirty)
{
	auto cb = GetBufferInfo(1); // Get PerObjectData buffer
	if (!cb || !cb->isPoolBuffer) return false;

	if (objectIndex >= cb->SubBufferOffsets.size())
	{
		std::cerr << "Error: Object index out of range in CopyPerObjectData!" << std::endl;
		return false;
	}

	size_t offset = cb->SubBufferOffsets[objectIndex];
	size_t objectDataSize = sizeof(DirectX::XMFLOAT4X4) * 2; // Two matrices

	// Only copy data if it is dirty (i.e., changed)
	if (isDirty)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		HRESULT hr = deviceContext->Map(
			cb->ConstantBuffer.Get(),
			0,
			D3D11_MAP_WRITE_NO_OVERWRITE,
			0,
			&mappedResource);

		if (FAILED(hr))
		{
			std::cerr << "Error: Failed to map PerObjectData buffer!" << std::endl;
			return false;
		}

		// Copy only the changed per-object data
		memcpy(
			static_cast<unsigned char*>(mappedResource.pData) + offset,
			cb->LocalDataBuffer + offset,
			objectDataSize
		);

		deviceContext->Unmap(cb->ConstantBuffer.Get(), 0);
	}

	// **Always bind the correct portion of the constant buffer**
	UINT firstConstant = (offset / 16);  // Ensure 16-byte alignment
	UINT numConstants = (PerObjectDataSizeAligned / 16);  // Enforce 256-byte alignment

	deviceContext->VSSetConstantBuffers1(
		cb->BindIndex,
		1,
		cb->ConstantBuffer.GetAddressOf(),
		&firstConstant,
		&numConstants);

	return true;
}


void LessSimpleVertexShader::SetShaderAndCBs()
{
	if (!shaderValid) return;

	deviceContext->IASetInputLayout(inputLayout.Get());
	deviceContext->VSSetShader(shader.Get(), nullptr, 0);

	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		if (constantBuffers[i].Type != D3D11_CT_CBUFFER)
			continue;

		// **Only set PerFrameData (register b0) here**
		if (constantBuffers[i].Name == "PerFrameData")
		{
			deviceContext->VSSetConstantBuffers(
				constantBuffers[i].BindIndex,
				1,
				constantBuffers[i].ConstantBuffer.GetAddressOf());
		}
	}
}

bool LessSimpleVertexShader::SetShaderResourceView(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	// Look for the variable and verify
	const SimpleSRV* srvInfo = GetShaderResourceViewInfo(name);
	if (srvInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("LessSimpleVertexShader::SetShaderResourceView() - SRV named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}
	// Set the shader resource view
	deviceContext->VSSetShaderResources(srvInfo->BindIndex, 1, srv.GetAddressOf());
	// Success
	return true;
}

bool LessSimpleVertexShader::SetSamplerState(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	// Look for the variable and verify
	const SimpleSampler* sampInfo = GetSamplerInfo(name);
	if (sampInfo == 0)
	{
		if (ReportWarnings)
		{
			LogWarning("LessSimpleVertexShader::SetSamplerState() - Sampler named '");
			Log(name);
			LogWarning("' was not found in the shader. Ensure the name is spelled correctly and that it exists in the shader.\n");
		}
		return false;
	}
	// Set the shader resource view
	deviceContext->VSSetSamplers(sampInfo->BindIndex, 1, samplerState.GetAddressOf());
	// Success
	return true;
}

///////////////////////////////////////////////////////////////////////////////
