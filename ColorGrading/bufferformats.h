#pragma once

#include "gl.h"

/*
Color buffers are described by 1 GLenum value as mapped in ColorBufferFormat (feel free to cast)
but some functions require information derived from the actual format. To derive these GLenum values
we have the formatDataType and highLevelFormat helper functions.

formatDataType returns the individual channel element value type, for example GL_R16F -> HALF_FLOAT
highLevelForamt returns a simplified format, for example GL_RG16UI -> GL_RG_INTEGER
*/
enum class ColorBufferFormat : GLenum
{
	// These are most internal formats mapped to a valid group of settings:
	// internal type, channel type, possible data formats
	// glTexImage2D variants need the latter 2 to upload data.
	// It helps in validating that provided data indeed matches the internal type.
	R8 = GL_R8,
	R8_SNORM = GL_R8_SNORM,
	R16F = GL_R16F,
	R32F = GL_R32F,
	R8UI = GL_R8UI,
	R8I = GL_R8I,
	R16UI = GL_R16UI,
	R16I = GL_R16I,
	R32UI = GL_R32UI,
	R32I = GL_R32I,
	RG8 = GL_RG8,
	RG8_SNORM = GL_RG8_SNORM,
	RG16F = GL_RG16F,
	RG32F = GL_RG32F,
	RG8UI = GL_RG8UI,
	RG8I = GL_RG8I,
	RG16UI = GL_RG16UI,
	RG16I = GL_RG16I,
	RG32UI = GL_RG32UI,
	RG32I = GL_RG32I,
	RGB8 = GL_RGB8,
	SRGB8 = GL_SRGB8,
	RGB565 = GL_RGB565,
	RGB8_SNORM = GL_RGB8_SNORM,
	R11F_G11F_B10F = GL_R11F_G11F_B10F,
	RGB9_E5 = GL_RGB9_E5,
	RGB16F = GL_RGB16F,
	RGB32F = GL_RGB32F,
	RGB8UI = GL_RGB8UI,
	RGB8I = GL_RGB8I,
	RGB16UI = GL_RGB16UI,
	RGB16I = GL_RGB16I,
	RGB32UI = GL_RGB32UI,
	RGB32I = GL_RGB32I,
	RGBA8 = GL_RGBA8,
	SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
	RGBA8_SNORM = GL_RGBA8_SNORM,
	RGB5_A1 = GL_RGB5_A1,
	RGBA4 = GL_RGBA4,
	RGB10_A2 = GL_RGB10_A2,
	RGBA16F = GL_RGBA16F,
	RGBA32F = GL_RGBA32F,
	RGBA8UI = GL_RGBA8UI,
	RGBA8I = GL_RGBA8I,
	RGB10_A2UI = GL_RGB10_A2UI,
	RGBA16UI = GL_RGBA16UI,
	RGBA16I = GL_RGBA16I,
	RGBA32I = GL_RGBA32I,
	RGBA32UI = GL_RGBA32UI,
	DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
	DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
	DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
	DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
	DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
};

GLenum highLevelFormat(ColorBufferFormat format);
int highLevelFormatChannels(GLenum highLevelFormat);
GLenum formatDataType(ColorBufferFormat format);

enum class RenderBufferFormat : GLenum
{
	R8 = GL_R8,
	R8UI = GL_R8UI,
	R8I = GL_R8I,
	R16UI = GL_R16UI,
	R16I = GL_R16I,
	R32UI = GL_R32UI,
	R32I = GL_R32I,
	RG8 = GL_RG8,
	RG8UI = GL_RG8UI,
	RG8I = GL_RG8I,
	RG16UI = GL_RG16UI,
	RG16I = GL_RG16I,
	RG32UI = GL_RG32UI,
	RG32I = GL_RG32I,
	RGB8 = GL_RGB8,
	RGB565 = GL_RGB565,
	RGBA8 = GL_RGBA8,
	SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,
	RGB5_A1 = GL_RGB5_A1,
	RGBA4 = GL_RGBA4,
	RGB10_A2 = GL_RGB10_A2,
	RGBA8UI = GL_RGBA8UI,
	RGBA8I = GL_RGBA8I,
	RGB10_A2UI = GL_RGB10_A2UI,
	RGBA16UI = GL_RGBA16UI,
	RGBA16I = GL_RGBA16I,
	RGBA32I = GL_RGBA32I,
	RGBA32UI = GL_RGBA32UI,
	DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
	DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
	DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
	DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
	DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
	STENCIL_INDEX8 = GL_STENCIL_INDEX8
};
