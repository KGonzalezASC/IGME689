#include "SimpleShader.h"

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
ISimpleShader::ISimpleShader(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Save the device
	this->device = device;
	this->deviceContext = context;

	// Set up fields
	this->constantBufferCount = 0;
	this->constantBuffers = 0;
	this->shaderValid = false;
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
ISimpleShader::~ISimpleShader()
{
	// Derived class destructors will call this class's CleanUp method
}

// --------------------------------------------------------
// Cleans up the variable table and buffers - Some things will
// be handled by derived classes
// --------------------------------------------------------
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

// --------------------------------------------------------
// Loads the specified shader and builds the variable table 
// using shader reflection.
//
// shaderFile - A "wide string" specifying the compiled shader to load
// 
// Returns true if shader is loaded properly, false otherwise
// --------------------------------------------------------
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

// --------------------------------------------------------
// Helper for looking up a constant buffer by name
// --------------------------------------------------------
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

// --------------------------------------------------------
// Prints the specified message to the console with the 
// given color and Visual Studio's output window
// --------------------------------------------------------
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

// --------------------------------------------------------
// Prints the specified message, as a wide string, to the 
// console with the given color and Visual Studio's output window
// --------------------------------------------------------
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


// --------------------------------------------------------
// Sets the shader and associated constant buffers in Direct3D
// --------------------------------------------------------
void ISimpleShader::SetShader()
{
	// Ensure the shader is valid
	if (!shaderValid) return;

	// Set the shader and any relevant constant buffers, which
	// is an overloaded method in a subclass
	SetShaderAndCBs();
}

// --------------------------------------------------------
// Copies the relevant data to the all of this 
// shader's constant buffers.  To just copy one
// buffer, use CopyBufferData()
// --------------------------------------------------------
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
	if(index >= this->constantBufferCount)
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
//
// bufferName - Specifies the name of the buffer to copy.
//              Useful for updating more frequently-changing
//              variables without having to re-copy all buffers.
// --------------------------------------------------------
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
	//finds the constant buffers to match the name uses the constantbufferindex to find the correct buffer to copy the data to
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

	//POSSIBLY FLAG HERE FOR PER FRAME, PER OBJECT, PER MATERIAL

	// Set the data in the local data buffer
	memcpy(
		constantBuffers[var->ConstantBufferIndex].LocalDataBuffer + var->ByteOffset,
		data,
		size);

	// Success
	return true;
}

// --------------------------------------------------------
// Sets INTEGER data
// --------------------------------------------------------
bool ISimpleShader::SetInt(std::string name, int data)
{
	return this->SetData(name, (void*)(&data), sizeof(int));
}

// --------------------------------------------------------
// Sets a FLOAT variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat(std::string name, float data)
{
	return this->SetData(name, (void*)(&data), sizeof(float));
}

// --------------------------------------------------------
// Sets a FLOAT2 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat2(std::string name, const float data[2])
{
	return this->SetData(name, (void*)data, sizeof(float) * 2);
}

// --------------------------------------------------------
// Sets a FLOAT2 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat2(std::string name, const DirectX::XMFLOAT2 data)
{
	return this->SetData(name, &data, sizeof(float) * 2);
}

// --------------------------------------------------------
// Sets a FLOAT3 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat3(std::string name, const float data[3])
{
	return this->SetData(name, (void*)data, sizeof(float) * 3);
}

// --------------------------------------------------------
// Sets a FLOAT3 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat3(std::string name, const DirectX::XMFLOAT3 data)
{
	return this->SetData(name, &data, sizeof(float) * 3);
}

// --------------------------------------------------------
// Sets a FLOAT4 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat4(std::string name, const float data[4])
{
	return this->SetData(name, (void*)data, sizeof(float) * 4);
}

// --------------------------------------------------------
// Sets a FLOAT4 variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetFloat4(std::string name, const DirectX::XMFLOAT4 data)
{
	return this->SetData(name, &data, sizeof(float) * 4);
}

// --------------------------------------------------------
// Sets a MATRIX (4x4) variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetMatrix4x4(std::string name, const float data[16])
{
	return this->SetData(name, (void*)data, sizeof(float) * 16);
}

// --------------------------------------------------------
// Sets a MATRIX (4x4) variable by name in the local data buffer
// --------------------------------------------------------
bool ISimpleShader::SetMatrix4x4(std::string name, const DirectX::XMFLOAT4X4 data)
{
	return this->SetData(name, &data, sizeof(float) * 16);
}

// --------------------------------------------------------
// Determines if the shader contains the specified
// variable within one of its constant buffers
// --------------------------------------------------------
bool ISimpleShader::HasVariable(std::string name)
{
	return FindVariable(name, -1) != 0;
}

// --------------------------------------------------------
// Determines if the shader contains the specified SRV
// --------------------------------------------------------
bool ISimpleShader::HasShaderResourceView(std::string name)
{
	return GetShaderResourceViewInfo(name) != 0;
}

// --------------------------------------------------------
// Determines if the shader contains the specified sampler
// --------------------------------------------------------
bool ISimpleShader::HasSamplerState(std::string name)
{
	return GetSamplerInfo(name) != 0;
}

// --------------------------------------------------------
// Gets info about a shader variable, if it exists
// --------------------------------------------------------
const SimpleShaderVariable* ISimpleShader::GetVariableInfo(std::string name)
{
	return FindVariable(name, -1);
}

// --------------------------------------------------------
// Gets info about an SRV in the shader (or null)
//
// name - the name of the SRV
// --------------------------------------------------------
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


// --------------------------------------------------------
// Gets info about an SRV in the shader (or null)
//
// index - the index of the SRV
// --------------------------------------------------------
const SimpleSRV* ISimpleShader::GetShaderResourceViewInfo(unsigned int index)
{
	// Valid index?
	if (index >= shaderResourceViews.size()) return 0;

	// Grab the bind index
	return shaderResourceViews[index];
}


// --------------------------------------------------------
// Gets info about a sampler in the shader (or null)
// 
// name - the name of the sampler
// --------------------------------------------------------
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

// --------------------------------------------------------
// Gets info about a sampler in the shader (or null)
// 
// index - the index of the sampler
// --------------------------------------------------------
const SimpleSampler* ISimpleShader::GetSamplerInfo(unsigned int index)
{
	// Valid index?
	if (index >= samplerStates.size()) return 0;

	// Grab the bind index
	return samplerStates[index];
}


// --------------------------------------------------------
// Gets the number of constant buffers in this shader
// --------------------------------------------------------
unsigned int ISimpleShader::GetBufferCount() { return constantBufferCount; }



// --------------------------------------------------------
// Gets the size of a particular constant buffer, or -1
// --------------------------------------------------------
unsigned int ISimpleShader::GetBufferSize(unsigned int index)
{
	// Valid index?
	if (index >= constantBufferCount)
		return -1;

	// Grab the size
	return constantBuffers[index].Size;
}

// --------------------------------------------------------
// Gets info about a particular constant buffer 
// by name, if it exists
// --------------------------------------------------------
const SimpleConstantBuffer * ISimpleShader::GetBufferInfo(std::string name)
{
	return FindConstantBuffer(name);
}

// --------------------------------------------------------
// Gets info about a particular constant buffer 
//
// index - the index of the constant buffer
// --------------------------------------------------------
const SimpleConstantBuffer * ISimpleShader::GetBufferInfo(unsigned int index)
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
SimpleVertexShader::SimpleVertexShader(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, LPCWSTR shaderFile)
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
SimpleVertexShader::SimpleVertexShader(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, LPCWSTR shaderFile, Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout, bool perInstanceCompatible)
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
	// Clean up first, in case this method is called multiple times
	this->CleanUp();

	// Create the vertex shader
	HRESULT result = device->CreateVertexShader(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		nullptr,
		shader.GetAddressOf());

	if (FAILED(result))
		return false;

	// If an input layout was provided in the constructor, use it
	if (inputLayout)
		return true;

	// Reflect shader info
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> refl;
	D3DReflect(
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(refl.GetAddressOf()));

	// Get shader description
	D3D11_SHADER_DESC shaderDesc;
	refl->GetDesc(&shaderDesc);

	// Input layout description
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		refl->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Detect per-instance data based on known semantic name
		bool isInstanceData = (std::string(paramDesc.SemanticName) == "INSTANCE_TRANSFORM");

		if (isInstanceData)
		{
			elementDesc.InputSlot = 1; // Instance data goes in a separate slot
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1; // Step once per instance
		}
		else
		{
			elementDesc.InputSlot = 0; // Standard per-vertex data
		}

		// Determine DXGI format
		if (paramDesc.Mask == 1)
		{
			elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		inputLayoutDesc.push_back(elementDesc);

		// Special handling for 4x4 matrices (INSTANCE_TRANSFORM)
		if (isInstanceData)
		{
			for (int j = 1; j < 4; j++)
			{
				D3D11_INPUT_ELEMENT_DESC matrixElement = elementDesc;
				matrixElement.SemanticIndex = j;
				matrixElement.AlignedByteOffset = j * sizeof(DirectX::XMFLOAT4);
				inputLayoutDesc.push_back(matrixElement);
			}
		}
	}

	// Create the input layout
	HRESULT hr = device->CreateInputLayout(
		inputLayoutDesc.data(),
		(UINT)inputLayoutDesc.size(),
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(),
		inputLayout.GetAddressOf());

	return SUCCEEDED(hr);
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

	// Binds the constant buffers, GPU knows which buffer to use for shader at bindIndex
	for (unsigned int i = 0; i < constantBufferCount; i++)
	{
		// Skip "buffers" that aren't true constant buffers
		if (constantBuffers[i].Type != D3D11_CT_CBUFFER || !constantBuffers[i].isDirty)
			continue;
		////all buffers are dirty as this time
		//if (constantBuffers[i].isDirty)
		//{
		//	Log("Buffer is dirty\n");
		//}
		//else
		//{
		//	Log("Buffer is clean and has not changed\n");
		//}

		// This is a real constant buffer, so set it
		deviceContext->VSSetConstantBuffers(
			constantBuffers[i].BindIndex,
			1,
			constantBuffers[i].ConstantBuffer.GetAddressOf());
	}
}

// --------------------------------------------------------
// Sets a shader resource view in the vertex shader stage
//
// name - The name of the texture resource in the shader
// srv - The shader resource view of the texture in GPU memory
//
// Returns true if a texture of the given name was found, false otherwise
// --------------------------------------------------------
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

// --------------------------------------------------------
// Sets a sampler state in the vertex shader stage
//
// name - The name of the sampler state in the shader
// samplerState - The sampler state in GPU memory
//
// Returns true if a sampler of the given name was found, false otherwise
// --------------------------------------------------------
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

// --------------------------------------------------------
// Constructor just calls the base
// --------------------------------------------------------
SimplePixelShader::SimplePixelShader(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, LPCWSTR shaderFile)
	: ISimpleShader(device, context) 
{ 
	// Load the actual compiled shader file
	this->LoadShaderFile(shaderFile);
}

// --------------------------------------------------------
// Destructor - Clean up actual shader (base will be called automatically)
// --------------------------------------------------------
SimplePixelShader::~SimplePixelShader()
{
	CleanUp();
}

// --------------------------------------------------------
// Handles cleaning up shader and base class clean up
// --------------------------------------------------------
void SimplePixelShader::CleanUp()
{
	ISimpleShader::CleanUp();
}

// --------------------------------------------------------
// Creates the  Direct3D pixel shader
//
// shaderBlob - The shader's compiled code
//
// Returns true if shader is created correctly, false otherwise
// --------------------------------------------------------
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

// --------------------------------------------------------
// Sets the pixel shader and constant buffers for
// future  Direct3D drawing
// --------------------------------------------------------
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

// --------------------------------------------------------
// Sets a shader resource view in the pixel shader stage
//
// name - The name of the texture resource in the shader
// srv - The shader resource view of the texture in GPU memory
//
// Returns true if a texture of the given name was found, false otherwise
// --------------------------------------------------------
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

// --------------------------------------------------------
// Sets a sampler state in the pixel shader stage
//
// name - The name of the sampler state in the shader
// samplerState - The sampler state in GPU memory
//
// Returns true if a sampler of the given name was found, false otherwise
// --------------------------------------------------------
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

