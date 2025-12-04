#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <vector>
#include "Barrier.h"
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler.lib")

class GPUFence {
public:
    ID3D12Fence* fence;
    HANDLE eventHandle;
    UINT64 value = 0;
    void create(ID3D12Device5* device) {
        device->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    void signal(ID3D12CommandQueue* queue) {
        queue->Signal(fence, ++value);
    }
    void wait() {
        if (fence->GetCompletedValue() < value) {
            fence->SetEventOnCompletion(value, eventHandle);
            WaitForSingleObject(eventHandle, INFINITE);
        }
    }
    ~GPUFence() {
        CloseHandle(eventHandle);
        fence->Release();
    }
};


class Core {
public:
	HWND _hwnd = nullptr;
	int _width = 0;
	int _height = 0;

	// Adapter
	IDXGIAdapter1* adapter = nullptr;

	// device
	ID3D12Device5* device = nullptr;

	// command queue
	ID3D12CommandQueue* graphicsQueue = nullptr;
	ID3D12CommandQueue* copyQueue = nullptr;
	ID3D12CommandQueue* computeQueue = nullptr;

	// swapchain
	IDXGISwapChain3* swapchain = nullptr;

	// back buffers
	ID3D12DescriptorHeap* backbufferHeap = nullptr;
	ID3D12Resource** backbuffers = nullptr;

	// command allocators
	ID3D12CommandAllocator* graphicsCommandAllocator[2];
	
	// command lists
	ID3D12GraphicsCommandList4* graphicsCommandList[2];

    // GPUFence
    GPUFence graphicsQueueFence[2];

    // depth buffer
    ID3D12DescriptorHeap* dsvHeap;
    ID3D12Resource* dsv;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

    // Viewport
    D3D12_VIEWPORT viewport;

    // Scissor
    D3D12_RECT scissorRect;

    // rootSignature
    ID3D12RootSignature* rootSignature = nullptr;

    // Release D3D12 Resource
    ~Core() {
        flushGraphicsQueue();

        rootSignature->Release();
        graphicsCommandList[0]->Release();
        graphicsCommandAllocator[0]->Release();
        graphicsCommandList[1]->Release();
        graphicsCommandAllocator[1]->Release();
        swapchain->Release();
        computeQueue->Release();
        copyQueue->Release();
        graphicsQueue->Release();
        device->Release();
        dsv->Release();
        dsvHeap->Release();
    }


    Core(HWND hwnd, int width, int height) : _hwnd(hwnd), _width(width), _height(height) {
        // Debug
        ID3D12Debug1* debug;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        debug->EnableDebugLayer();
        debug->Release();

        // Enumerate adapters
        IDXGIAdapter1* adapterf;
        std::vector<IDXGIAdapter1*> adapters;
        IDXGIFactory4* factory = NULL;
        CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)&factory);
        int i = 0;
        while (factory->EnumAdapters1(i, &adapterf) != DXGI_ERROR_NOT_FOUND)
        {
            adapters.push_back(adapterf);
            i++;
        }

        // Find the best adapter
        long long maxVideoMemory = 0;
        int bestIndex = 0;

        for (int i = 0; i < (int)adapters.size(); ++i)
        {
            DXGI_ADAPTER_DESC desc;
            adapters[i]->GetDesc(&desc);

            if (desc.DedicatedVideoMemory > maxVideoMemory)
            {
                maxVideoMemory = desc.DedicatedVideoMemory;
                bestIndex = i;
            }
        }

        adapter = adapters[bestIndex];

        // create Device
        D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));

        // Create Command Queues
        D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc = {};
        graphicsQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        device->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&graphicsQueue));
        D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
        copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&copyQueue));
        D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
        computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&computeQueue));

        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.Width = _width;
        scDesc.Height = _height;
        scDesc.SampleDesc.Count = 1; // MSAA here
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferCount = 2;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        // Create the swapchain
        IDXGISwapChain1* swapChain1;
        factory->CreateSwapChainForHwnd(graphicsQueue, _hwnd, &scDesc, NULL, NULL, &swapChain1);
        swapChain1->QueryInterface(&swapchain);
        swapChain1->Release();
        factory->Release();

        // Create Command Allocators and Command Lists
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&graphicsCommandAllocator[0]));
        device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
            IID_PPV_ARGS(&graphicsCommandList[0]));
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&graphicsCommandAllocator[1]));
        device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE,
            IID_PPV_ARGS(&graphicsCommandList[1]));

        // Create Heap in init
        D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc = {};
        renderTargetViewHeapDesc.NumDescriptors = scDesc.BufferCount;
        renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&backbufferHeap));

        // Allocate memory for Backbuffer array
        backbuffers = new ID3D12Resource * [scDesc.BufferCount];

        // Get backbuffers and create views on heap
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = backbufferHeap->GetCPUDescriptorHandleForHeapStart();
        unsigned int renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (unsigned int i = 0; i < 2; i++)
        {
            swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
            device->CreateRenderTargetView(backbuffers[i], nullptr, renderTargetViewHandle);
            renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
        }

        // Create fences
        graphicsQueueFence[0].create(device);
        graphicsQueueFence[1].create(device);

        // ============================
        // Create Depth Buffer
        // ============================

        // Create Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        memset(&dsvHeapDesc, 0, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
        dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

        // Fill in structs
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
        D3D12_CLEAR_VALUE depthClearValue = {};
        depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.DepthStencil.Stencil = 0;

        // Want fast on chip memory
        D3D12_HEAP_PROPERTIES heapprops = {};
        heapprops.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapprops.CreationNodeMask = 1;
        heapprops.VisibleNodeMask = 1;

        // Specify resource information
        D3D12_RESOURCE_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Width = width;
        dsvDesc.Height = height;
        dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        dsvDesc.DepthOrArraySize = 1;
        dsvDesc.MipLevels = 1;
        dsvDesc.SampleDesc.Count = 1;
        dsvDesc.SampleDesc.Quality = 0;
        dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        // Allocate memory for Resource
        device->CreateCommittedResource(&heapprops, D3D12_HEAP_FLAG_NONE, &dsvDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&dsv));
        // Create DepthStencilView to Resource on descriptor heap
        device->CreateDepthStencilView(dsv, &depthStencilDesc, dsvHandle);

        // ============================
        // Viewport & Scissor
        // ============================
        
        // Define viewport and scissor rect
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (float)width;
        viewport.Height = (float)height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = width;
        scissorRect.bottom = height;

        // ============================
        // Root Signature
        // ============================
        std::vector<D3D12_ROOT_PARAMETER> parameters;

        D3D12_ROOT_PARAMETER rootParameterCBVS = {};
        rootParameterCBVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameterCBVS.Descriptor.ShaderRegister = 0; // Register(b0)
        rootParameterCBVS.Descriptor.RegisterSpace = 0;
        rootParameterCBVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        parameters.push_back(rootParameterCBVS);

        D3D12_ROOT_PARAMETER rootParameterCBPS = {};
        rootParameterCBPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameterCBPS.Descriptor.ShaderRegister = 0; // Register(b0)
        rootParameterCBPS.Descriptor.RegisterSpace = 0;
        rootParameterCBPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        parameters.push_back(rootParameterCBPS);

        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = (UINT)parameters.size();
        rsDesc.pParameters = parameters.data();
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ID3DBlob* serialized = nullptr;
        ID3DBlob* error = nullptr;
        D3D12SerializeRootSignature(
            &rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);

        device->CreateRootSignature(
            0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature));

        serialized->Release();

    }

    // reset command allocator and command list
    void resetCommandList()
    {
        unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
        graphicsCommandAllocator[frameIndex]->Reset();
        graphicsCommandList[frameIndex]->Reset(graphicsCommandAllocator[frameIndex], NULL);
    }

    // get current Command List
    ID3D12GraphicsCommandList4* getCommandList()
    {
        unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
        return graphicsCommandList[frameIndex];
    }

    // Close and execute the list
    void runCommandList()
    {
        getCommandList()->Close();
        ID3D12CommandList* lists[] = { getCommandList() };
        graphicsQueue->ExecuteCommandLists(1, lists);
    }

    void beginFrame() {
        // Get Backbuffer to draw to
        // Find Backbuffer index
        unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
        
        // Wait for previous commands to finish
        // Ensure the GPU has finished and present current backbuffer
        graphicsQueueFence[frameIndex].wait();
        resetCommandList();

        // Find RenderTargetView at index
        // - Find value from heap
        // - Increment by index
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = backbufferHeap -> GetCPUDescriptorHandleForHeapStart();
        unsigned int renderTargetViewDescriptorSize = device -> GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        renderTargetViewHandle.ptr += frameIndex * renderTargetViewDescriptorSize;

        // Bind Viewport + Scissor
        getCommandList()->RSSetViewports(1, &viewport);
        getCommandList()->RSSetScissorRects(1, &scissorRect);

        // Clear Backbuffer and Depth Buffer
        // Issue commands on the command list

        Barrier::add(backbuffers[frameIndex], D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET, getCommandList());
        getCommandList()->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, &dsvHandle);
        float Colour[4];
        Colour[0] = 0;
        Colour[1] = 0;
        Colour[2] = 1.0;
        Colour[3] = 1.0;
        getCommandList()->ClearRenderTargetView(renderTargetViewHandle, Colour, 0, NULL);
        getCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
    }

    void finishFrame()
    {
        unsigned int frameIndex = swapchain->GetCurrentBackBufferIndex();
        Barrier::add(backbuffers[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT, getCommandList());
        runCommandList();
        graphicsQueueFence[frameIndex].signal(graphicsQueue);
        swapchain->Present(1, 0);
    }


    // Ensures all work completed before moving on
    void flushGraphicsQueue() {
        graphicsQueueFence[0].signal(graphicsQueue);
        graphicsQueueFence[0].wait();
    }

    void uploadResource(ID3D12Resource* dstResource, 
                        const void* data, unsigned int size,
                        D3D12_RESOURCE_STATES targetState, 
                        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* texFootprint = NULL) {

        // Allocate memory in upload heap
        ID3D12Resource* uploadBuffer;
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = size;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));

        // Get pointer to allocated memory on upload heap
        void* mappeddata = NULL;
        uploadBuffer->Map(0, NULL, &mappeddata);

        // Memcpy vertex data
        memcpy(mappeddata, data, size);

        // Tell driver we are done
        uploadBuffer->Unmap(0, NULL);

        // Allocate commands to copy
        resetCommandList();

        // Issue copy command
        if (texFootprint != NULL)
        {
            D3D12_TEXTURE_COPY_LOCATION src = {};
            src.pResource = uploadBuffer;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = *texFootprint;
            D3D12_TEXTURE_COPY_LOCATION dst = {};
            dst.pResource = dstResource;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = 0;
            getCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);
        }
        else
        {
            getCommandList()->CopyBufferRegion(dstResource, 0, uploadBuffer, 0, size);
        }

        // Transition buffer to final state after copying
        Barrier::add(dstResource, D3D12_RESOURCE_STATE_COPY_DEST, targetState, getCommandList());

        // Close and execute command lists
        runCommandList();

        // Wait for the command to finish
        flushGraphicsQueue();

        // Release upload heap memory
        uploadBuffer->Release();

    }

    int frameIndex()
    {
        return swapchain->GetCurrentBackBufferIndex();
    }

    void beginRenderPass()
    {
        getCommandList()->RSSetViewports(1, &viewport);
        getCommandList()->RSSetScissorRects(1, &scissorRect);
        getCommandList()->SetGraphicsRootSignature(rootSignature);
    }
};