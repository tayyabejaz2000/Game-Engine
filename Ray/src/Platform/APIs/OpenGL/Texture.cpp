#include "stb_image.h"
#include "Platform/APIs/OpenGL/Texture.hpp"

namespace Ray::OpenGL
{
    uint32_t TextureSpecificationToGLTypes::s_formatLookup[] = {
        [(uint32_t)Ray::Texture::Format::R8] = GL_R8,
        [(uint32_t)Ray::Texture::Format::R8I] = GL_R8I,
        [(uint32_t)Ray::Texture::Format::R32F] = GL_R32F,

        [(uint32_t)Ray::Texture::Format::RG8] = GL_RG8,
        [(uint32_t)Ray::Texture::Format::RG8I] = GL_RG8I,
        [(uint32_t)Ray::Texture::Format::RG32F] = GL_RG32F,

        [(uint32_t)Ray::Texture::Format::RGB8] = GL_RGB8,
        [(uint32_t)Ray::Texture::Format::RGB32F] = GL_RGB32F,

        [(uint32_t)Ray::Texture::Format::RGBA8] = GL_RGBA8,
        [(uint32_t)Ray::Texture::Format::RGBA16] = GL_RGBA16,
        [(uint32_t)Ray::Texture::Format::RGBA16F] = GL_RGBA16F,
        [(uint32_t)Ray::Texture::Format::RGB32I] = GL_RGB32I,
        [(uint32_t)Ray::Texture::Format::RGBA32F] = GL_RGBA32F,

        [(uint32_t)Ray::Texture::Format::STENCIL8] = GL_STENCIL_INDEX8,
        [(uint32_t)Ray::Texture::Format::DEPTH24STENCIL8] = GL_DEPTH24_STENCIL8,
        [(uint32_t)Ray::Texture::Format::DEPTH32] = GL_DEPTH_COMPONENT32F, //Check this either GL_DEPTH or GL_DEPTH_COMPONENT32F
    };

    uint32_t TextureSpecificationToGLTypes::GetTextureWrapping(const Ray::Texture::Wrap &wrapping)
    {
        switch (wrapping)
        {
        case Ray::Texture::Wrap::REPEAT:
            return GL_REPEAT;
        case Ray::Texture::Wrap::MIRRORED_REPEAT:
            return GL_MIRRORED_REPEAT;
        case Ray::Texture::Wrap::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case Ray::Texture::Wrap::MIRROR_CLAMP_TO_EDGE:
            return GL_MIRROR_CLAMP_TO_EDGE;
        case Ray::Texture::Wrap::CLAMP_TO_BORDER:
            return GL_CLAMP_TO_BORDER;
        }
        return GL_NONE;
    }
    uint32_t TextureSpecificationToGLTypes::GetTextureDataFormat(const Ray::Texture::Format &format)
    {
        switch (format)
        {
        case Ray::Texture::Format::R8... Ray::Texture::Format::R32F:
            return GL_RED;
        case Ray::Texture::Format::RG8... Ray::Texture::Format::RG32F:
            return GL_RG;
        case Ray::Texture::Format::RGB8... Ray::Texture::Format::RGB32F:
            return GL_RGB;
        case Ray::Texture::Format::RGBA8... Ray::Texture::Format::RGBA32F:
            return GL_RGBA;
        case Ray::Texture::Format::STENCIL8:
            return GL_STENCIL;
        case Ray::Texture::Format::DEPTH24STENCIL8:
            return GL_DEPTH_STENCIL;
        case Ray::Texture::Format::DEPTH32:
            return GL_DEPTH;
        };
        return GL_NONE;
    }

    Texture2D::Texture2D(const Texture2DSpecification &specs) : m_specs(specs)
    {
        auto internalFormat = TextureSpecificationToGLTypes::GetTextureFormat(m_specs.Format);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
        glTextureStorage2D(m_rendererID, 1, internalFormat, m_specs.Width, m_specs.Height);

        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, TextureSpecificationToGLTypes::GetTextureWrapping(m_specs.WrappingS));
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, TextureSpecificationToGLTypes::GetTextureWrapping(m_specs.WrappingT));
    }
    Texture2D::Texture2D(const std::string &path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        auto data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data)
            throw std::runtime_error("Failed to load Image");

        m_specs.Width = width;
        m_specs.Height = height;
        uint32_t internalFormat = 0, dataFormat = 0;
        switch (channels)
        {
        case 3:
            dataFormat = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            dataFormat = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        case 2:
            dataFormat = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 1:
            dataFormat = GL_RED;
            internalFormat = GL_R8;
            break;
        }

        if (!(internalFormat & dataFormat))
            throw std::runtime_error("Format not Supported");

        glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
        glTextureStorage2D(m_rendererID, 1, internalFormat, m_specs.Width, m_specs.Height);

        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(m_rendererID, 0, 0, 0, m_specs.Width, m_specs.Height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
    Texture2D::~Texture2D()
    {
        glDeleteTextures(1, &m_rendererID);
    }

    void Texture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_rendererID);
    }

    void Texture2D::SetData(void *data, uint32_t size)
    {
        auto dataFormat = TextureSpecificationToGLTypes::GetTextureDataFormat(m_specs.Format);
        ///TODO: Get Size for each data format
        auto bpp = (dataFormat == GL_RGBA) ? 4u : 3u;
        ///Warning: No check for now
        //if (size != m_specs.Width * m_specs.Height * bpp)
        //    throw std::runtime_error("Data must be entire Texture");
        glTextureSubImage2D(m_rendererID, 0, 0, 0, m_specs.Width, m_specs.Height, dataFormat, GL_UNSIGNED_BYTE, data);
    }

    void Texture2D::Clear(uint32_t value)
    {
        glClearTexImage(m_rendererID, 0,
                        TextureSpecificationToGLTypes::GetTextureFormat(m_specs.Format),
                        GL_INT, &value);
    }
}