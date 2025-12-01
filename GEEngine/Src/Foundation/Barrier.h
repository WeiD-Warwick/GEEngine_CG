#pragma once
#include <d3d12.h>

class Barrier {
public:
	static void add(ID3D12Resource* res, D3D12_RESOURCE_STATES first, D3D12_RESOURCE_STATES second,
		ID3D12GraphicsCommandList4* commandList) {
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Transition.pResource = res;
		rb.Transition.StateBefore = first;
		rb.Transition.StateAfter = second;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		commandList->ResourceBarrier(1, &rb);
	}
};
