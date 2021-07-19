#pragma once

#include "core/core_types.h"

#include <d3d12.h>
// Do all drivers support this??
#include <dxgi1_6.h>
#include <d3dcompiler.h>
// TODO: make my own math library.
#include <DirectXMath.h>

#include "renderer/d3dx12.h"

// @Cleanup: Don't link these libs in source code.
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Renderer
{
	static const u32 FRAME_COUNT = 2;

	// Pipeline objects.
	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissor_rect;
	IDXGISwapChain1* swap_chain;
	ID3D12Device* device;
	ID3D12Resource* render_targets[FRAME_COUNT];
	ID3D12CommandAllocator* command_allocator;
	ID3D12CommandQueue* command_queue;
	ID3D12RootSignature* root_signature;
	ID3D12DescriptorHeap* rtv_descriptor_heap;
	ID3D12PipelineState* pipeline_state;
	ID3D12GraphicsCommandList* command_list;
	u32 rtv_descriptor_size = 0;

	// App resources.
	ID3D12Resource* vertex_buffer;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;

	// Synchronization objects.
	u32 frame_index = 0;
	HANDLE fence_event;
	ID3D12Fence* frame_fence;
	u64 fence_value;

	// TEMPORARY
	f32 aspect_ratio;

	bool initialize(u32 viewport_width, u32 viewport_height, HWND hwnd);
	void update();
	void render();
	void shutdown();

	void load_pipeline(u32 viewport_width, u32 viewport_height, HWND hwnd);
	void load_assets();
	void populate_command_list();
	void wait_for_previous_frame(bool increment_frame);

	void get_hardware_adapter(IDXGIFactory1* factory, IDXGIAdapter1** adapter, bool request_high_performance_adapter = false);
};