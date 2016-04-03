#ifndef __GraphicsDevice_h__
#define __GraphicsDevice_h__

#include "CameraClearFlags.h"
#include "Color.h"
#include <memory>
#include <vector>

#ifdef ANDROID
#include <GLES2/gl2.h>
#endif

namespace Galaxy3D
{
    class RenderTexture;
    class Texture;
    class Material;
    class Mesh;
    class Matrix4x4;
    struct VertexShader;

    struct BufferUsage
    {
        enum Enum
        {
            StaticDraw,
            DynamicDraw
        };
    };

    struct BufferType
    {
        enum Enum
        {
            Vertex,
            Index
        };
    };

    struct IndexType
    {
        enum Enum
        {
            UShort,
            UInt
        };
    };

    struct BufferObject
    {
        void *buffer;
		BufferUsage::Enum usage;
		BufferType::Enum type;

        BufferObject():buffer(NULL) {}
    };

	class GraphicsDevice
	{
	public:
		static GraphicsDevice *GetInstance();
        static void Done();
		static void CheckGetError(const char *file, int line);
		~GraphicsDevice();
		void Init(void *param);
		void ClearShaderResources();
        std::weak_ptr<RenderTexture> GetScreenBuffer() const {return m_screen_buffer;}
        void Blit(const std::shared_ptr<Texture> &source, const std::shared_ptr<RenderTexture> &destination, const std::shared_ptr<Material> &material, int pass);
        void DrawMeshNow(const std::shared_ptr<Mesh> &mesh, int sub_mesh_index, const std::shared_ptr<Material> &material, int pass_index);
        void SetViewport(int left, int top, int width, int height);
        void Present();
        void SetRenderTargets(const std::vector<std::shared_ptr<RenderTexture>> &color_buffers, const std::shared_ptr<RenderTexture> &depth_stencil_buffer);
        void ClearRenderTarget(CameraClearFlags::Enum clear_flags, const Color &color, float depth, int stencil);
        BufferObject CreateBufferObject(void *data, int size, BufferUsage::Enum usage, BufferType::Enum type);
        void UpdateBufferObject(BufferObject &bo, void *data, int size);
        void ReleaseBufferObject(BufferObject &bo);
        void SetInputLayout(VertexShader *shader);
        void SetVertexBuffer(BufferObject &bo, VertexShader *shader);
        void SetIndexBuffer(BufferObject &bo, IndexType::Enum bits);
        void DrawIndexed(int count, int offset);
		bool IsRenderTargetScreen() const {return m_frame_buffer_current == 0;}

	private:
		GLuint m_frame_buffer_current;
        std::shared_ptr<RenderTexture> m_screen_buffer;
        std::shared_ptr<Mesh> m_blit_mesh;
        std::shared_ptr<Material> m_blit_mat;
        std::vector<std::weak_ptr<RenderTexture>> m_color_buffers_ref;
        std::weak_ptr<RenderTexture> m_depth_stencil_buffer_ref;
		IndexType::Enum m_index_buffer_bits;

        GraphicsDevice();
        void CreateBlitMeshIfNeeded();
        void CreateBlitMaterialIfNeeded();
	};

#define CHECK_GL_ERROR GraphicsDevice::CheckGetError(__FILE__, __LINE__)
}

#endif