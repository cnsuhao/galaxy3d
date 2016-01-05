#include "GraphicsDevice.h"
#include "Screen.h"
#include "RenderTexture.h"
#include "Mesh.h"

namespace Galaxy3D
{
    static GraphicsDevice *g_device = NULL;

	GraphicsDevice *GraphicsDevice::GetInstance()
	{
        if(g_device == NULL)
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
            Screen::GetWidth(),//UINT Width;
            Screen::GetHeight(),//UINT Height;
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
        cam->SetRenderTarget(destination, false);

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

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();
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
                context->IASetInputLayout(pass->vs->input_layout);
                UINT stride = pass->vs->vertex_stride;
                UINT offset = 0;
                context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
                context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);
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

        auto context = GraphicsDevice::GetInstance()->GetDeviceContext();

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

            UINT stride = pass->vs->vertex_stride;
            UINT offset = 0;
            context->IASetInputLayout(pass->vs->input_layout);
            context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
            context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R16_UINT, 0);

            material->ReadyPass(pass_index);
            pass->rs->Apply();
            material->ApplyPass(pass_index);

            Renderer::DrawIndexed(index_count, index_offset);
        }while(false);
    }
}