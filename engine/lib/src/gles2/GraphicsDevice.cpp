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
		m_frame_buffer_current(0)
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
		ClearShaderResources();

		m_screen_buffer.reset();
        m_blit_mesh.reset();
        m_blit_mat.reset();
	}

	void GraphicsDevice::Init(void *param)
	{
		m_screen_buffer = std::shared_ptr<RenderTexture>(
            new RenderTexture(Screen::GetWidth(), Screen::GetHeight(), 0));
	}

	void GraphicsDevice::ClearShaderResources()
	{
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
                SetVertexBuffer(vertex_buffer, pass->vs);
                SetIndexBuffer(index_buffer, IndexType::UShort);
            }

            mat->ReadyPass(i);
            pass->rs->Apply();
            mat->ApplyPass(i);

            Renderer::DrawIndexed(6, 0);
        }

		ClearShaderResources();
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

            auto pass = shader->GetPass(pass_index);

            SetInputLayout(pass->vs);
            SetVertexBuffer(vertex_buffer, pass->vs);
            SetIndexBuffer(index_buffer, IndexType::UShort);

            material->ReadyPass(pass_index);
            pass->rs->Apply();
            material->ApplyPass(pass_index);

            Renderer::DrawIndexed(index_count, index_offset);
        }while(false);
    }

	void GraphicsDevice::SetViewport(int left, int top, int width, int height)
	{
		glViewport(left, top, width, height);
	}

	void GraphicsDevice::Present()
	{
	}

	void GraphicsDevice::SetRenderTargets(const std::vector<std::shared_ptr<RenderTexture>> &color_buffers, const std::shared_ptr<RenderTexture> &depth_stencil_buffer)
    {
		std::shared_ptr<RenderTexture> target;

		if(!color_buffers.empty())
		{
			target = color_buffers[0];
		}
		else
		{
			target = depth_stencil_buffer;
		}
		
		auto frame_buffer = target->GetFrameBuffer();
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		m_frame_buffer_current = frame_buffer;
    }

	void GraphicsDevice::ClearRenderTarget(CameraClearFlags::Enum clear_flags, const Color &color, float depth, int stencil)
	{
		if(clear_flags == CameraClearFlags::SolidColor)
        {
            glClearColor(color.r, color.g, color.b, color.a);
			glClearDepthf(depth);
			glClearStencil(stencil);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        else if(clear_flags == CameraClearFlags::Depth)
        {
            glClearDepthf(depth);
			glClearStencil(stencil);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
	}

	BufferObject GraphicsDevice::CreateBufferObject(void *data, int size, BufferUsage::Enum usage, BufferType::Enum type)
	{
		GLenum target;
		GLenum use;
		
		if(type == BufferType::Vertex)
		{
			target = GL_ARRAY_BUFFER;
		}
		else
		{
			target = GL_ELEMENT_ARRAY_BUFFER;
		}

		if(usage == BufferUsage::StaticDraw)
		{
			use = GL_STATIC_DRAW;
		}
		else
		{
			use = GL_DYNAMIC_DRAW;
		}

		GLuint buffer = 0;
		glGenBuffers(1, &buffer);
        glBindBuffer(target, buffer);
        glBufferData(target, size, data, use);
        glBindBuffer(target, 0);

		BufferObject bo;
		bo.buffer = (void *) buffer;
		bo.type = type;
		bo.usage = usage;
        return bo;
	}

	void GraphicsDevice::UpdateBufferObject(BufferObject &bo, void *data, int size)
	{
		GLenum target;
		GLenum use;
		
		if(bo.type == BufferType::Vertex)
		{
			target = GL_ARRAY_BUFFER;
		}
		else
		{
			target = GL_ELEMENT_ARRAY_BUFFER;
		}

		if(bo.usage == BufferUsage::StaticDraw)
		{
			use = GL_STATIC_DRAW;
		}
		else
		{
			use = GL_DYNAMIC_DRAW;
		}

		GLuint buffer = (GLuint) bo.buffer;

		glBindBuffer(target, buffer);
		glBufferData(target, size, data, use);
		glBindBuffer(target, 0);
	}

	void GraphicsDevice::ReleaseBufferObject(BufferObject &bo)
	{
		GLuint buffer = (GLuint) bo.buffer;

		glDeleteBuffers(1, &buffer);
		bo.buffer = 0;
	}

	void GraphicsDevice::SetInputLayout(VertexShader *shader)
	{
	}

	void GraphicsDevice::SetVertexBuffer(BufferObject &bo, VertexShader *shader)
	{
		GLuint buffer = (GLuint) bo.buffer;

		glBindBuffer(GL_ARRAY_BUFFER, buffer);

        for(auto &i : shader->attributes)
        {
            int slot = i.slot;
            int size = i.size;
			int offset = i.offset;
            glVertexAttribPointer(slot, size, GL_FLOAT, GL_FALSE, shader->vertex_stride, (const GLvoid *) offset);
            glEnableVertexAttribArray(slot);
        }
	}

	void GraphicsDevice::SetIndexBuffer(BufferObject &bo, IndexType::Enum bits)
	{
		GLuint buffer = (GLuint) bo.buffer;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

		m_index_buffer_bits = bits;
	}

	void GraphicsDevice::DrawIndexed(int count, int offset)
	{
		GLenum type;

		if(m_index_buffer_bits == IndexType::UShort)
		{
			type = GL_UNSIGNED_SHORT;
		}
		else
		{
			type = GL_UNSIGNED_INT;
		}

		glDrawElements(GL_TRIANGLES, count, type, (const GLvoid *) offset);
	}
}