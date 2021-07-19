#include "renderer/renderer.h"

#include "renderer/d3d12_helpers.h"

// TEMPORARY
struct Vertex
{
	f32 postion[4];
	f32 color[4];
};

bool Renderer::initialize(u32 viewport_width, u32 viewport_height, HWND hwnd)
{
	// Maybe do this in constructor.
	viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<f32>(viewport_width), static_cast<f32>(viewport_height));
	scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(viewport_width), static_cast<LONG>(viewport_height));
	aspect_ratio = (f32)viewport_width / (f32)viewport_height;

	load_pipeline(viewport_width, viewport_height, hwnd);
	load_assets();
	return true;
}

void Renderer::update()
{

}

void Renderer::render()
{
	// Record all the commands to render a single frame.
	populate_command_list();

	// Execute the command list.
	ID3D12CommandList* command_lists[] = { command_list };
	command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// Present the frame.
	ThrowIfFailed(swap_chain->Present(1, 0));

	wait_for_previous_frame(true);
}

void Renderer::shutdown()
{
	// Ensure that the GPU is no longer referencing resources that are about to be cleaned up.
	wait_for_previous_frame(false);

	CloseHandle(fence_event);
}

void Renderer::load_pipeline(u32 viewport_width, u32 viewport_height, HWND hwnd)
{
	u32 dxgi_factory_flags = 0;

#if defined(RENDERER_DEBUG)
	// TODO: Turning on the debug layer may be useful in non debug builds. Something
	// to consider for the future.
	{
		ID3D12Debug* debug_controller;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
		{
			debug_controller->EnableDebugLayer();

			// Enable additional debug layers.
			dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory)));

	// Find a hardware adapter.
	IDXGIAdapter1* hardware_adapter;
	get_hardware_adapter(factory, &hardware_adapter);

	// Create D3D12Device
	ThrowIfFailed(D3D12CreateDevice(hardware_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// Create command queue;
	ThrowIfFailed(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.BufferCount        = FRAME_COUNT;
	swap_chain_desc.Width              = viewport_width;
	swap_chain_desc.Height             = viewport_height;
	swap_chain_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.SampleDesc.Count   = 1;
	swap_chain_desc.SampleDesc.Quality = 0;

	// NOTE: we are using a IDXGISwapChain1. I need to look at what the other
	// numbers do.
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		command_queue,
		hwnd,
		&swap_chain_desc,
		nullptr,
		nullptr,
		&swap_chain
	));

	// TODO: Create fullscreen swapchain and maybe support HDR.
	ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	// Create descriptor heaps.
	{
		// RTV descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtv_desc_heap = {};
		rtv_desc_heap.NumDescriptors = FRAME_COUNT;
		rtv_desc_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_desc_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device->CreateDescriptorHeap(&rtv_desc_heap, IID_PPV_ARGS(&rtv_descriptor_heap)));

		rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

		Assert(rtv_descriptor_size > 0);

		for (u32 frame_index = 0; frame_index < FRAME_COUNT; ++frame_index)
		{
			ThrowIfFailed(swap_chain->GetBuffer(frame_index, IID_PPV_ARGS(&render_targets[frame_index])));
			device->CreateRenderTargetView(render_targets[frame_index], nullptr, rtv_handle);
			rtv_handle.Offset(1, rtv_descriptor_size);
		}
	}

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));
}

void Renderer::load_assets()
{
	// Create and empty root signature.
	{
		CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
		root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob* signature;
		ID3DBlob* error;
		ThrowIfFailed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ID3DBlob* vertex_shader;
		ID3DBlob* pixel_shader;

#if defined(RENDERER_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		u32 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		u32 compile_flags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(L"F:/Dev/d3d12_renderer/assets/shaders/simple_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"F:/Dev/d3d12_renderer/assets/shaders/simple_ps.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
		pso_desc.pRootSignature = root_signature;
		pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader);
		pso_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader);
		pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pso_desc.DepthStencilState.DepthEnable = FALSE;
		pso_desc.DepthStencilState.StencilEnable = FALSE;
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pso_desc.SampleDesc.Count = 1;
		ThrowIfFailed(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));
	}

	// Create command list.
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr, IID_PPV_ARGS(&command_list)));

	// Command lists are created in the recording state, but there is nothing to record... yet.
	// The main loop expects it to be closed, so close it for now.
	ThrowIfFailed(command_list->Close());

	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex triangle_vertices[] =
		{
			{ { 0.0f, 0.4f * aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.4f, -0.4f * aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.4f, -0.4f * aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const u32 vertex_buffer_size = sizeof(triangle_vertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertex_buffer)));

		// Copy the triangle data to the vertex buffer.
		u8* vertex_data_begin;
		CD3DX12_RANGE read_range(0, 0); // We do not intend to read from this buffer on the CPU.
		ThrowIfFailed(vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin)));
		memcpy(vertex_data_begin, triangle_vertices, sizeof(triangle_vertices));
		vertex_buffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
		vertex_buffer_view.StrideInBytes  = sizeof(Vertex);
		vertex_buffer_view.SizeInBytes    = vertex_buffer_size;
	}

	// Create synchronization objects.
	{
		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frame_fence)));
		fence_value = 1;

		// Create an event handle to use for frame synchronization.
		fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fence_event == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		wait_for_previous_frame(false);
	}
}

void Renderer::populate_command_list()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(command_allocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(command_list->Reset(command_allocator, pipeline_state));

	// Set necessary state.
	command_list->SetGraphicsRootSignature(root_signature);
	command_list->RSSetViewports(1, &viewport);
	command_list->RSSetScissorRects(1, &scissor_rect);

	// Indicate that the back buffer will be used as a render target.
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets[frame_index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);
	command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	command_list->ClearRenderTargetView(rtv_handle, clearColor, 0, nullptr);
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
	command_list->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets[frame_index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(command_list->Close());
}

void Renderer::wait_for_previous_frame(bool increment_frame)
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const u64 fence = fence_value;
	ThrowIfFailed(command_queue->Signal(frame_fence, fence));
	fence_value++;

	// Wait until the previous frame is finished.
	if (frame_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(frame_fence->SetEventOnCompletion(fence, fence_event));
		WaitForSingleObject(fence_event, INFINITE);
	}

	if (increment_frame)
	{
		// TODO: SwapChain3 has a useful method to do this for us.
		frame_index = (frame_index + 1) % 2;
	}
}

void Renderer::get_hardware_adapter(IDXGIFactory1* factory, IDXGIAdapter1** adapter, bool request_high_performance_adapter)
{
	*adapter = nullptr;

	IDXGIAdapter1* possible_adapter;

	IDXGIFactory6* factory6;
	if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (u32 adapter_index = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
				adapter_index,
				request_high_performance_adapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&possible_adapter));
			++adapter_index)
		{
			DXGI_ADAPTER_DESC1 desc;
			possible_adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the
				// command line.
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(possible_adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}
	else
	{
		for (u32 adapter_index = 0;
			DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &possible_adapter);
			++adapter_index)
		{
			DXGI_ADAPTER_DESC1 desc;
			possible_adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the
				// command line.
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(possible_adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*adapter = possible_adapter;
}