#pragma once

#include "core/core_types.h"

#include <initguid.h>
#include "renderer/d3d12_headers.h"

#include "renderer/d3dx12.h"

// @Cleanup: Don't link these libs in source code.
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct SceneConstantBuffer
{
	DirectX::XMFLOAT4 offset;
	float padding[60]; // Padding so the constant buffer is 256-byte aligned.
};

struct VertexXUV
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned.");

struct Renderer
{
	static const u32 FRAME_COUNT = 2;
	static const u32 TEXTURE_WIDTH = 256;
	static const u32 TEXTURE_HEIGHT = 256;
	static const u32 TEXTURE_PIXEL_SIZE = 4; // The number of bytes used to represent a pixel in the texture.

	// Pipeline objects.
	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissor_rect;
	IDXGISwapChain1* swap_chain;
	ID3D12Device* device;
	ID3D12Resource* render_targets[FRAME_COUNT];
	ID3D12CommandAllocator* command_allocator;
	ID3D12CommandQueue* command_queue;
	ID3D12RootSignature* empty_root_signature;
	ID3D12RootSignature* single_texture_root_signature;
	ID3D12DescriptorHeap* rtv_descriptor_heap;
	ID3D12DescriptorHeap* srv_descriptor_heap;
	ID3D12PipelineState* pipeline_state;
	ID3D12GraphicsCommandList* command_list;
	u32 rtv_descriptor_size = 0;

	// App resources.
	ID3D12Resource* vertex_buffer;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
	ID3D12Resource* texture;
	ID3D12Resource* constant_buffer;
	SceneConstantBuffer constant_buffer_data;
	UINT8* cbv_data_begin = nullptr;

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

	static std::vector<u8> generate_texture_data();
};