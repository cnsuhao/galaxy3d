#include "GraphicsDevice.h"
#include "Screen.h"
#include "RenderTexture.h"
#include "Mesh.h"

namespace Galaxy3D
{
    static GraphicsDevice *g_device = NULL;
    static bool g_done = false;

	GraphicsDevice *GraphicsDevice::GetInstance()
	{
        if(g_device == NULL && !g_done)
        {
            g_device = new GraphicsDevice();
        }

		return g_device;
	}

    void GraphicsDevice::Done()
    {
        if(g_device != NULL)
        {
            delete g_device;
            g_device = NULL;
        }

        g_done = true;
    }

	GraphicsDevice::GraphicsDevice():
		m_d3d_device(NULL),
		m_swap_chain(NULL),
		m_immediate_context(NULL),
		m_render_target_view(NULL),
		m_depth_stencil_texture(NULL),
		m_depth_stencil_view(NULL)
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
        m_screen_buffer.reset();
        m_blit_mesh.reset();
        m_blit_mat.reset();

        ClearShaderResources();
        m_immediate_context->VSSetShader(NULL, NULL, 0);
        ID3D11Buffer *cb[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {NULL};
        m_immediate_context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cb);
        m_immediate_context->PSSetShader(NULL, NULL, 0);
        m_immediate_context->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, cb);
        m_immediate_context->IASetInputLayout(NULL);
        ID3D11Buffer *vb = NULL;
        UINT zero = 0;
        m_immediate_context->IASetVertexBuffers(0, 1, &vb, &zero, &zero);
        m_immediate_context->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
        m_immediate_context->RSSetState(NULL);
        m_immediate_context->OMSetBlendState(NULL, 0, 0xffffffff);
        m_immediate_context->OMSetDepthStencilState(NULL, 0);
        ID3D11RenderTargetView *view[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {NULL};
        m_immediate_context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, view, NULL);
        m_immediate_context->Flush();
        m_immediate_context->ClearState();

        SAFE_RELEASE(m_depth_stencil_view);
        SAFE_RELEASE(m_depth_stencil_texture);
        SAFE_RELEASE(m_render_target_view);
        SAFE_RELEASE(m_immediate_context);
        SAFE_RELEASE(m_swap_chain);
        
#if _DEBUG
        ID3D11Debug *debug;
        HRESULT hr = m_d3d_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
        SAFE_RELEASE(m_d3d_device);

        if(SUCCEEDED(hr))
        {
            hr = debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        }
        SAFE_RELEASE(debug);
#else
        SAFE_RELEASE(m_d3d_device);
#endif
	}

	void GraphicsDevice::Init(void *param)
	{
		HRESULT hr = S_OK;

		UINT flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
		flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HWND hwnd = (HWND) param;

		//device and swap chain
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			//D3D_DRIVER_TYPE_WARP,
			//D3D_DRIVER_TYPE_REFERENCE,
		};
		int numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			//D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		int numFeatureLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = Screen::GetWidth();
		sd.BufferDesc.Height = Screen::GetHeight();
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for(int i=0; i<numDriverTypes; i++)
		{
			auto driverType = driverTypes[i];
			D3D_FEATURE_LEVEL level;
			hr = D3D11CreateDeviceAndSwapChain(
				NULL, driverType, NULL, flag,
				featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_d3d_device, &level, &m_immediate_context);
			if(SUCCEEDED(hr))
				break;
		}
		if(FAILED(hr))
			return;

		//defaut render target 
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer);
		if(FAILED(hr))
			return;

		hr = m_d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &m_render_target_view);
		pBackBuffer->Release();
		if(FAILED(hr))
			return;

		//set primitive topology
		m_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//depth stencil texture
        D3D11_TEXTURE2D_DESC dtd =
        {
            (UINT) Screen::GetWidth(),//UINT Width;
            (UINT) Screen::GetHeight(),//UINT Height;
            1,//UINT MipLevels;
            1,//UINT ArraySize;
            DXGI_FORMAT_D24_UNORM_S8_UINT,//DXGI_FORMAT Format;
            1, 0,//DXGI_SAMPLE_DESC SampleDesc;
            D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
            D3D11_BIND_DEPTH_STENCIL,//UINT BindFlags;
            0,//UINT CPUAccessFlags;
            0//UINT MiscFlags;
        };
		m_d3d_device->CreateTexture2D(&dtd, NULL, &m_depth_stencil_texture);
		m_d3d_device->CreateDepthStencilView(m_depth_stencil_texture, NULL, &m_depth_stencil_view);

        m_screen_buffer = std::shared_ptr<RenderTexture>(
            new RenderTexture(Screen::GetWidth(), Screen::GetHeight(), m_render_target_view, m_depth_stencil_view));
	}

	void GraphicsDevice::ClearShaderResources()
	{
		ID3D11ShaderResourceView *sr[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {NULL};
		m_immediate_context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, sr);

        ID3D11SamplerState *s[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {NULL};
        m_immediate_context->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, s);
	}

    void GraphicsDevice::CreateBlitMeshIfNeeded()
    {
        if(!m_blit_mesh)
        {
            m_blit_mesh = Mesh::Create();

            std::vector<VertexMesh> vertices;
            std::vector<std::vector<unsigned short>> indices(1);
            indices[0].resize(6);

            VertexMesh v0 = {Vector3(-1, 1, 0), Vector3(), Vector4(), Vector2(0, 0), Vector2()};
            VertexMesh v1 = {Vector3(1, 1, 0.3f), Vector3(), Vector4(), Vector2(1, 0), Vector2()};
            VertexMesh v2 = {Vector3(1, -1, 0.2f), Vector3(), Vector4(), Vector2(1, 1), Vector2()};
            VertexMesh v3 = {Vector3(-1, -1, 0.1f), Vector3(), Vector4(), Vector2(0, 1), Vector2()};

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);

            unsigned short index_buffer[] = {0, 1, 2, 0, 2, 3};
            memcpy(&indices[0][0], index_buffer, sizeof(index_buffer));

            m_blit_mesh->SetVertices(vertices);
            m_blit_mesh->SetIndices(indices);
        }
    }

    void GraphicsDevice::CreateBlitMaterialIfNeeded()
    {
        if(!m_blit_mat)
        {
            m_blit_mat = Material::Create("BlitCopy");
        }
    }

    void GraphicsDevice::Blit(const std::shared_ptr<Texture> &source, const std::shared_ptr<RenderTexture> &destination, const std::shared_ptr<Material> &material, int pass)
    {
        CreateBlitMeshIfNeeded();

        auto cam = Camera::GetCurrent();
        cam->SetRenderTarget(destination, false, false);

        std::shared_ptr<Material> mat;
        if(material)
        {
            mat = material;
        }
        else
        {
            CreateBlitMaterialIfNeeded();

            mat = m_blit_mat;
        }
        mat->SetMainTexture(source);

        auto vertex_buffer = m_blit_mesh->GetVertexBuffer();
        auto index_buffer = m_blit_mesh->GetIndexBuffer();
        auto shader = mat->GetShader();

        int pass_begin;
        int pass_end;
        if(pass >= 0)
        {
            pass_begin = pass;
            pass_end = pass;
        }
        else
        {
            pass_begin = 0;
            pass_end = shader->GetPassCount() - 1;
        }

        for(int i = pass_begin; i <= pass_end && i < shader->GetPassCount(); i++)
        {
            auto pass = shader->GetPass(i);

            if(i == pass_begin)
            {
                SetInputLayout(pass->vs);
                SetVertexBuffer(vertex_buffer, pass->vs->vertex_stride, 0);
                SetIndexBuffer(index_buffer, IndexType::UShort);
            }

            mat->ReadyPass(i);
            pass->rs->Apply();
            mat->ApplyPass(i);

            Renderer::DrawIndexed(6, 0);
        }

        GraphicsDevice::GetInstance()->ClearShaderResources();
    }

    void GraphicsDevice::DrawMeshNow(const std::shared_ptr<Mesh> &mesh, int sub_mesh_index, const std::shared_ptr<Material> &material, int pass_index)
    {
        auto vertex_buffer = mesh->GetVertexBuffer();
        auto index_buffer = mesh->GetIndexBuffer();

        int index_offset = 0;
        for(int i=0; i<sub_mesh_index; i++)
        {
            index_offset += mesh->GetIndexCount(i);
        }

        do
        {
            int index_count = mesh->GetIndexCount(sub_mesh_index);
            auto shader = material->GetShader();

            auto pass_count = shader->GetPassCount();
            auto pass = shader->GetPass(pass_index);

            SetInputLayout(pass->vs);
            SetVertexBuffer(vertex_buffer, pass->vs->vertex_stride, 0);
            SetIndexBuffer(index_buffer, IndexType::UShort);

            material->ReadyPass(pass_index);
            pass->rs->Apply();
            material->ApplyPass(pass_index);

            Renderer::DrawIndexed(index_count, index_offset);
        }while(false);
    }

    void GraphicsDevice::SetViewport(int left, int top, int width, int height)
    {
        D3D11_VIEWPORT vp;
        vp.Width = (float) width;
        vp.Height = (float) height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = (float) left;
        vp.TopLeftY = (float) top;

        m_immediate_context->RSSetViewports(1, &vp);
    }

    void GraphicsDevice::Present()
    {
        m_swap_chain->Present(0, 0);
    }

    void GraphicsDevice::SetRenderTargets(const std::vector<std::shared_ptr<RenderTexture>> &color_buffers, const std::shared_ptr<RenderTexture> &depth_stencil_buffer)
    {
        int color_count = color_buffers.size();
        m_color_buffers_ref.resize(color_count);
        m_depth_stencil_buffer_ref = depth_stencil_buffer;
        std::vector<ID3D11RenderTargetView *> colors(color_count);
        for(int i=0; i<color_count; i++)
        {
            colors[i] = color_buffers[i]->GetRenderTargetView();
            m_color_buffers_ref[i] = color_buffers[i];
        }
        ID3D11DepthStencilView *depth_stencil = NULL;
        if(depth_stencil_buffer)
        {
            depth_stencil = depth_stencil_buffer->GetDepthStencilView();
        }

        if(color_count > 0)
        {
            m_immediate_context->OMSetRenderTargets(color_count, &colors[0], depth_stencil);
        }
        else
        {
            m_immediate_context->OMSetRenderTargets(0, NULL, depth_stencil);
        }
    }

    void GraphicsDevice::ClearRenderTarget(CameraClearFlags::Enum clear_flags, const Color &color, float depth, int stencil)
    {
        if(clear_flags == CameraClearFlags::SolidColor)
        {
            for(size_t i=0; i<m_color_buffers_ref.size(); i++)
            {
                auto color_buffer = m_color_buffers_ref[i].lock()->GetRenderTargetView();

                if(color_buffer != NULL)
                {
                    m_immediate_context->ClearRenderTargetView(color_buffer, (const float *) &color);
                }
            }

            ID3D11DepthStencilView *depth_stencil = NULL;
            if(!m_depth_stencil_buffer_ref.expired())
            {
                depth_stencil = m_depth_stencil_buffer_ref.lock()->GetDepthStencilView();;
            }

            if(depth_stencil != NULL)
            {
                m_immediate_context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
            }
        }
        else if(clear_flags == CameraClearFlags::Depth)
        {
            ID3D11DepthStencilView *depth_stencil = NULL;
            if(!m_depth_stencil_buffer_ref.expired())
            {
                depth_stencil = m_depth_stencil_buffer_ref.lock()->GetDepthStencilView();;
            }

            if(depth_stencil != NULL)
            {
                m_immediate_context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
            }
        }
    }

    BufferObject GraphicsDevice::CreateBufferObject(void *data, int size, BufferUsage::Enum usage, BufferType::Enum type)
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.ByteWidth = size;

        if(type == BufferType::Vertex)
        {
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        else if(type == BufferType::Index)
        {
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        }

        if(usage == BufferUsage::StaticDraw)
        {
            bd.Usage = D3D11_USAGE_IMMUTABLE;
            bd.CPUAccessFlags = 0;
        }
        else if(usage == BufferUsage::DynamicDraw)
        {
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }

        D3D11_SUBRESOURCE_DATA init_data;
        ZeroMemory(&init_data, sizeof(init_data));
        init_data.pSysMem = data;

        ID3D11Buffer *buffer = NULL;
        m_d3d_device->CreateBuffer(&bd, &init_data, &buffer);

        BufferObject bo;
        bo.buffer = buffer;
        return bo;
    }

    void GraphicsDevice::UpdateBufferObject(BufferObject &bo, void *data, int size)
    {
        ID3D11Buffer *buffer = (ID3D11Buffer *) bo.buffer;

        D3D11_MAPPED_SUBRESOURCE dms;
        ZeroMemory(&dms, sizeof(dms));
        m_immediate_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dms);
        memcpy(dms.pData, data, size);
        m_immediate_context->Unmap(buffer, 0);
    }

    void GraphicsDevice::ReleaseBufferObject(BufferObject &bo)
    {
        ID3D11Buffer *buffer = (ID3D11Buffer *) bo.buffer;
        SAFE_RELEASE(buffer);
        bo.buffer = 0;
    }

    void GraphicsDevice::SetInputLayout(VertexShader *shader)
    {
        m_immediate_context->IASetInputLayout(shader->input_layout);
    }

    void GraphicsDevice::SetVertexBuffer(BufferObject &bo, int stride, int offset)
    {
        UINT strides[1] = {(UINT) stride};
        UINT offsets[1] = {(UINT) offset};
        ID3D11Buffer *buffer = (ID3D11Buffer *) bo.buffer;
        m_immediate_context->IASetVertexBuffers(0, 1, &buffer, strides, offsets);
    }

    void GraphicsDevice::SetIndexBuffer(BufferObject &bo, IndexType::Enum bits)
    {
        ID3D11Buffer *buffer = (ID3D11Buffer *) bo.buffer;
        DXGI_FORMAT format;
        if(bits == IndexType::UInt)
        {
            format = DXGI_FORMAT_R32_UINT;
        }
        else
        {
            format = DXGI_FORMAT_R16_UINT;
        }

        m_immediate_context->IASetIndexBuffer(buffer, format, 0);
    }

    void GraphicsDevice::DrawIndexed(int count, int offset)
    {
        m_immediate_context->DrawIndexed(count, offset, 0);
    }
}