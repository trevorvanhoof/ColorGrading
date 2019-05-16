#pragma once

#include "bufferformats.h"
#include "alerts.h"

class GraphicsHandleBase
{
protected:
	GLint* _handle = nullptr;
	virtual void _initialize() = 0;
	virtual void _uninitialize() {};

public:
	template<typename T>
	T handle()
	{
		if (_handle == nullptr)
			_initialize();
		return *_handle;
	}

	virtual ~GraphicsHandleBase() { _uninitialize(); }
};

class BufferObject2DBase : public GraphicsHandleBase
{
protected:
	GLenum _internalFormat;
	int _width;
	int _height;

	virtual void _sizeChanged() = 0;

public:
	BufferObject2DBase(GLenum internalFormat, int width, int height);
	virtual void setSize(int width, int height);

	// getters
	// inline GLenum format() { return _internalFormat; }
	inline int width() { return _width; }
	inline int height() { return _height; }
};

class ColorBufferObject2DBase : public BufferObject2DBase
{
protected:
	std::vector<std::vector<unsigned char>> _data;
	int _mipLevels;
	bool _tiling;

	virtual GLenum _textureType() = 0;

	virtual void _sizeChanged() override;
	virtual void _initialize() override;
	virtual void _uninitialize() override;
	void _tilingSet();
	void _generateMipMaps();

	virtual int _numPixels(int factor) { return (_width * _height) / (factor * factor); }

	template<typename T>
	T* _read(GLenum format, int mipLevel = 0)
	{
		int factor = 1 << mipLevel;
		int bufferSize = highLevelFormatChannels(highLevelFormat((ColorBufferFormat)_internalFormat)) * _numPixels(factor);
		T* result = new T[bufferSize];
		bind();
		glGetTexImage(_textureType(), mipLevel, highLevelFormat((ColorBufferFormat)_internalFormat), format, result);
		return result;
	}

public:
	ColorBufferObject2DBase(ColorBufferFormat internalFormat, int width, int height, std::vector<std::vector<unsigned char>> dataPerMipLevel);

	virtual void setSize(int width, int height) override;

	// getters
	inline ColorBufferFormat format() { return (ColorBufferFormat)_internalFormat; }
	inline bool hasMips() { return _mipLevels > 1; }
	inline int mipLevels() { return _mipLevels; }
	inline bool tiling() { return _tiling; }

	void setTiling(bool tiling);
	inline void bind() { glBindTexture(_textureType(), handle<GLuint>()); }
	void generateMipMaps(int levels = 0);
	void bindLoadStore(GLenum layout, GLenum mode = GL_WRITE_ONLY);

	inline float* readFloats(int mipLevel = 0) { return _read<float>(GL_FLOAT, mipLevel); }
	inline unsigned char* readBytes(int mipLevel = 0) { return _read<unsigned char>(GL_UNSIGNED_BYTE, mipLevel); }
};

class ColorBufferObject2D : public ColorBufferObject2DBase
{
protected:
	inline virtual GLenum _textureType() override { return GL_TEXTURE_2D; }

public:
	ColorBufferObject2D(ColorBufferFormat internalFormat, int width, int height, std::vector<std::vector<unsigned char>> dataPerMipLevel);
	static ColorBufferObject2D fromQImage(const QImage& img, bool tile = false, bool srgb = false);
	QImage toQImage(int mipLevel = 0);
};

class ColorBufferObject3D : public ColorBufferObject2DBase
{
protected:
	int _depth;
	inline virtual GLenum _textureType() override { return GL_TEXTURE_3D; }
	virtual int _numPixels(int factor) override { return (_width * _height * _depth) / (factor * factor * factor); }
	virtual void _sizeChanged() override;

public:
	ColorBufferObject3D(ColorBufferFormat internalFormat, int width, int height, int depth, std::vector<std::vector<unsigned char>> dataPerMipLevel);

	inline int depth() { return _depth; }
	
	void setSize(int width, int height, int depth);
};

class RenderBufferObject : public BufferObject2DBase
{
protected:
	virtual void _sizeChanged() override;
	virtual void _initialize() override;
	virtual void _uninitialize() override;

public:
	RenderBufferObject(RenderBufferFormat internalFormat, int width, int height);
	inline RenderBufferFormat format() { return (RenderBufferFormat)_internalFormat; }
	void bind();
};

class ShaderStorageBufferObject : public GraphicsHandleBase
{
protected:
	int _size;
	char* _data = nullptr;
	
	virtual void _initialize() override;
	virtual void _uninitialize() override;

public:
	ShaderStorageBufferObject(int size, char* data = nullptr);
	virtual ~ShaderStorageBufferObject();
	void setSize(int size);
	void setData(int dataSize, char* data);
	void bind(int index);
	static void unbindAll();

	inline int sizeInBytes() { return _size; }

	template<typename T> T* read()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, *(GLuint*)_handle);
		char* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		int bufferSize = _size / sizeof(T);
		T* buffer = new T[bufferSize];
		CopyMemory(buffer, ptr, _size);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return buffer;
	}
};

/*
Cube maps are a bit of a nightmare to get consistently working, especially in the data department
I opted for the data to contain large to small mip maps for every face, back to back.
*/
class ColorBufferObjectCube : public ColorBufferObject2DBase
{
protected:
	inline virtual GLenum _textureType() override { return GL_TEXTURE_CUBE_MAP; }

	virtual int _numPixels(int factor) { return (_width * _width * _width) / (factor * factor * factor); }

	template<typename T>
	T* _read(GLenum format, int face, int mipLevel = 0)
	{
		assert(0 <= mipLevel && mipLevel < _mipLevels);
		int factor = 1 << mipLevel;
		int bufferSize = highLevelFormatChannels(highLevelFormat((ColorBufferFormat)_internalFormat)) * _numPixels(factor);
		T* result = new T[bufferSize];
		bind();
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipLevel, highLevelFormat((ColorBufferFormat)_internalFormat), format, result);
		return result;
	}

	virtual void _sizeChanged() override;

public:
	ColorBufferObjectCube(ColorBufferFormat internalFormat, int size, std::vector<std::vector<unsigned char>> dataPerMipLevelPerFace);
	
	inline void setSize(int size) { setSize(size, size); }
	virtual void setSize(int width, int height) override;

	inline float* readFloats(int face, int mipLevel = 0) { return _read<float>(GL_FLOAT, face, mipLevel); }
	inline unsigned char* readBytes(int face, int mipLevel = 0) { return _read<unsigned char>(GL_UNSIGNED_BYTE, face, mipLevel); }
};
