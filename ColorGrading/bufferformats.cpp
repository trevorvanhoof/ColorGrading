#include "bufferformats.h"
#include "alerts.h"

int highLevelFormatChannels(GLenum highLevelFormat)
{
	switch (highLevelFormat)
	{
	case GL_RED:
	case GL_RED_INTEGER:
		return 1;
	case GL_RG:
	case GL_RG_INTEGER:
		return 2;
	case GL_RGB:
	case GL_RGB_INTEGER:
		return 3;
	case GL_RGBA:
	case GL_RGBA_INTEGER:
		return 4;
	case GL_DEPTH_COMPONENT:
		return 1;
	case GL_DEPTH_STENCIL:
		return 2;
	}
	error("Unexpected high level format, invalid format.");
	return 4;
}

GLenum highLevelFormat(ColorBufferFormat format)
{
	switch (format)
	{
	case ColorBufferFormat::R8: return GL_RED;
	case ColorBufferFormat::R8_SNORM: return GL_RED;
	case ColorBufferFormat::R16F: return GL_RED;
	case ColorBufferFormat::R32F: return GL_RED;
	case ColorBufferFormat::R8UI: return GL_RED_INTEGER;
	case ColorBufferFormat::R8I: return GL_RED_INTEGER;
	case ColorBufferFormat::R16UI: return GL_RED_INTEGER;
	case ColorBufferFormat::R16I: return GL_RED_INTEGER;
	case ColorBufferFormat::R32UI: return GL_RED_INTEGER;
	case ColorBufferFormat::R32I: return GL_RED_INTEGER;
	case ColorBufferFormat::RG8: return GL_RG;
	case ColorBufferFormat::RG8_SNORM: return GL_RG;
	case ColorBufferFormat::RG16F: return GL_RG;
	case ColorBufferFormat::RG32F: return GL_RG;
	case ColorBufferFormat::RG8UI: return GL_RG_INTEGER;
	case ColorBufferFormat::RG8I: return GL_RG_INTEGER;
	case ColorBufferFormat::RG16UI: return GL_RG_INTEGER;
	case ColorBufferFormat::RG16I: return GL_RG_INTEGER;
	case ColorBufferFormat::RG32UI: return GL_RG_INTEGER;
	case ColorBufferFormat::RG32I: return GL_RG_INTEGER;
	case ColorBufferFormat::RGB8: return GL_RGB;
	case ColorBufferFormat::SRGB8: return GL_RGB;
	case ColorBufferFormat::RGB565: return GL_RGB;
	case ColorBufferFormat::RGB8_SNORM: return GL_RGB;
	case ColorBufferFormat::R11F_G11F_B10F: return GL_RGB;
	case ColorBufferFormat::RGB9_E5: return GL_RGB;
	case ColorBufferFormat::RGB16F: return GL_RGB;
	case ColorBufferFormat::RGB32F: return GL_RGB;
	case ColorBufferFormat::RGB8UI: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGB8I: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGB16UI: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGB16I: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGB32UI: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGB32I: return GL_RGB_INTEGER;
	case ColorBufferFormat::RGBA8: return GL_RGBA;
	case ColorBufferFormat::SRGB8_ALPHA8: return GL_RGBA;
	case ColorBufferFormat::RGBA8_SNORM: return GL_RGBA;
	case ColorBufferFormat::RGB5_A1: return GL_RGBA;
	case ColorBufferFormat::RGBA4: return GL_RGBA;
	case ColorBufferFormat::RGB10_A2: return GL_RGBA;
	case ColorBufferFormat::RGBA16F: return GL_RGBA;
	case ColorBufferFormat::RGBA32F: return GL_RGBA;
	case ColorBufferFormat::RGBA8UI: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGBA8I: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGB10_A2UI: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGBA16UI: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGBA16I: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGBA32I: return GL_RGBA_INTEGER;
	case ColorBufferFormat::RGBA32UI: return GL_RGBA_INTEGER;
	case ColorBufferFormat::DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT;
	case ColorBufferFormat::DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT;
	case ColorBufferFormat::DEPTH_COMPONENT32F: return GL_DEPTH_COMPONENT;
	case ColorBufferFormat::DEPTH24_STENCIL8: return GL_DEPTH_STENCIL;
	case ColorBufferFormat::DEPTH32F_STENCIL8: return GL_DEPTH_STENCIL;
	}
	error("Unexpected ColorBufferFormat, either not all enum cases are handled or an illegal cast has happened.");
	return GL_RGBA;
}

GLenum formatDataType(ColorBufferFormat format)
{
	switch (format)
	{
	case ColorBufferFormat::R8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::R8_SNORM: return GL_BYTE;
	case ColorBufferFormat::R16F: return GL_HALF_FLOAT;
	case ColorBufferFormat::R32F: return GL_FLOAT;
	case ColorBufferFormat::R8UI: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::R8I: return GL_BYTE;
	case ColorBufferFormat::R16UI: return GL_UNSIGNED_SHORT;
	case ColorBufferFormat::R16I: return GL_SHORT;
	case ColorBufferFormat::R32UI: return GL_UNSIGNED_INT;
	case ColorBufferFormat::R32I: return GL_INT;
	case ColorBufferFormat::RG8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RG8_SNORM: return GL_BYTE;
	case ColorBufferFormat::RG16F: return GL_HALF_FLOAT;
	case ColorBufferFormat::RG32F: return GL_FLOAT;
	case ColorBufferFormat::RG8UI: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RG8I: return GL_BYTE;
	case ColorBufferFormat::RG16UI: return GL_UNSIGNED_SHORT;
	case ColorBufferFormat::RG16I: return GL_SHORT;
	case ColorBufferFormat::RG32UI: return GL_UNSIGNED_INT;
	case ColorBufferFormat::RG32I: return GL_INT;
	case ColorBufferFormat::RGB8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::SRGB8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGB565: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGB8_SNORM: return GL_BYTE;
	case ColorBufferFormat::R11F_G11F_B10F: return GL_UNSIGNED_INT_10F_11F_11F_REV;
	case ColorBufferFormat::RGB9_E5: return GL_UNSIGNED_INT_5_9_9_9_REV;
	case ColorBufferFormat::RGB16F: return GL_HALF_FLOAT;
	case ColorBufferFormat::RGB32F: return GL_FLOAT;
	case ColorBufferFormat::RGB8UI: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGB8I: return GL_BYTE;
	case ColorBufferFormat::RGB16UI: return GL_UNSIGNED_SHORT;
	case ColorBufferFormat::RGB16I: return GL_SHORT;
	case ColorBufferFormat::RGB32UI: return GL_UNSIGNED_INT;
	case ColorBufferFormat::RGB32I: return GL_INT;
	case ColorBufferFormat::RGBA8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::SRGB8_ALPHA8: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGBA8_SNORM: return GL_BYTE;
	case ColorBufferFormat::RGB5_A1: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGBA4: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGB10_A2: return GL_UNSIGNED_INT_2_10_10_10_REV;
	case ColorBufferFormat::RGBA16F: return GL_HALF_FLOAT;
	case ColorBufferFormat::RGBA32F: return GL_FLOAT;
	case ColorBufferFormat::RGBA8UI: return GL_UNSIGNED_BYTE;
	case ColorBufferFormat::RGBA8I: return GL_BYTE;
	case ColorBufferFormat::RGB10_A2UI: return GL_UNSIGNED_INT_2_10_10_10_REV;
	case ColorBufferFormat::RGBA16UI: return GL_UNSIGNED_SHORT;
	case ColorBufferFormat::RGBA16I: return GL_SHORT;
	case ColorBufferFormat::RGBA32I: return GL_INT;
	case ColorBufferFormat::RGBA32UI: return GL_UNSIGNED_INT;
	case ColorBufferFormat::DEPTH_COMPONENT16: return GL_UNSIGNED_SHORT;
	case ColorBufferFormat::DEPTH_COMPONENT24: return GL_UNSIGNED_INT;
	case ColorBufferFormat::DEPTH_COMPONENT32F: return GL_FLOAT;
	case ColorBufferFormat::DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
	case ColorBufferFormat::DEPTH32F_STENCIL8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	}
	error("Unexpected ColorBufferFormat, either not all enum cases are handled or an illegal cast has happened.");
	return GL_RGBA;
}
