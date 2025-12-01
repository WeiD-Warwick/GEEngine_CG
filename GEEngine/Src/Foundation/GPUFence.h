#pragma once
#include <d3d12.h>

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
