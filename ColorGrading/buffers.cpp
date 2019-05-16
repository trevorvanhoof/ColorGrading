#include "buffers.h"

BufferObject2DBase::BufferObject2DBase(GLenum internalFormat, int width, int height) :
	_internalFormat(internalFormat), _width(width), _height(height)
{
}

void BufferObject2DBase::setSize(int width, int height)
{
	if (width == _width && height == _height)
		return;
	_width = width;
	_height = height;
	if (_handle != nullptr)
		_sizeChanged();
}

void ColorBufferObject2DBase::_sizeChanged()
{
	// never called before initialize()

	// reallocate with new size
	glBindTexture(_textureType(), *(GLuint*)_handle);
	for (int mipLevel = 0; mipLevel < _mipLevels; ++mipLevel)
	{
		int factor = 1 << mipLevel;
		
		// because of glTexImage2D this function must be reimplemented in non-2D texture subclasses
		if (mipLevel >= _data.size())
			glTexImage2D(_textureType(), mipLevel, _internalFormat, _width / factor, _height / factor, 0, highLevelFormat(format()), formatDataType(format()), nullptr);
		else
			glTexImage2D(_textureType(), mipLevel, _internalFormat, _width / factor, _height / factor, 0, highLevelFormat(format()), formatDataType(format()), &_data[mipLevel][0]);
		// level 0 has been set, allocate mips before pushing more data
		if (!mipLevel && _data.size() > 1)
			_generateMipMaps();
	}
}

void ColorBufferObject2DBase::_initialize()
{
	_handle = new GLint[1];
	glGenTextures(1, (GLuint*)_handle);
	_sizeChanged();
	if (!hasMips())
		glTexParameteri(_textureType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	else
		glTexParameteri(_textureType(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	_tilingSet();
}

void ColorBufferObject2DBase::_uninitialize()
{
	glDeleteTextures(1, (GLuint*)_handle);
	delete[] _handle;
}

void ColorBufferObject2DBase::_tilingSet()
{
	bind();
	if (!_tiling)
	{
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // this has no impact on 2D textures so always setting it in the base makes things easier
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_R, GL_REPEAT);  // this has no impact on 2D textures so always setting it in the base makes things easier
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(_textureType(), GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

void ColorBufferObject2DBase::_generateMipMaps()
{
	bind();
	glTexParameteri(_textureType(), GL_TEXTURE_MAX_LEVEL, _mipLevels);
	gl.glGenerateMipmap(_textureType());
}

void ColorBufferObject2DBase::setTiling(bool tiling)
{
	_tiling = tiling;
	if (_handle)
		_tilingSet();
}

void ColorBufferObject2DBase::generateMipMaps(int levels)
{
	assert(_data.size() < 2, "Can not generate mip maps for a texture with data for multiple mip levels, the data for each mip has been set during initialization.");

	if (levels)
		_mipLevels = levels;
	else
	{
		// https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
		_mipLevels = 1;
		int tmp = (_width > _height) ? _width : _height;
		while (tmp >>= 1)++_mipLevels;
	}

	if (_handle)
		_generateMipMaps();
}

void ColorBufferObject2DBase::bindLoadStore(GLenum layout, GLenum mode)
{
	gl.glBindImageTexture(layout, handle<GLuint>(), 0, false, 0, mode, (GLenum)_internalFormat);
}

ColorBufferObject2DBase::ColorBufferObject2DBase(ColorBufferFormat internalFormat, int width, int height, std::vector<std::vector<unsigned char>> dataPerMipLevel) :
	BufferObject2DBase((GLenum)internalFormat, width, height), _data(dataPerMipLevel), _tiling(false), _mipLevels((int)dataPerMipLevel.size())
{
	if (!_mipLevels)
		_mipLevels = 1;
}

void ColorBufferObject2DBase::setSize(int width, int height)
{
	assertFatal(_data.size() == 0, "Can not resize a texture that was initialized with data. The data dictates the required resolution.");
	BufferObject2DBase::setSize(width, height);
}

ColorBufferObject2D::ColorBufferObject2D(ColorBufferFormat internalFormat, int width, int height, std::vector<std::vector<unsigned char>> dataPerMipLevel) :
	ColorBufferObject2DBase(internalFormat, width, height, dataPerMipLevel)
{
	_mipLevels = 1;
}

ColorBufferObject2D ColorBufferObject2D::fromQImage(const QImage& img, bool tile, bool srgb)
{
	std::vector<std::vector<unsigned char>> data;
	int numBytes = img.width() * img.height() * 4;
	data.push_back(std::vector<unsigned char>(numBytes));
	CopyMemory(&data[0][0], img.bits(), numBytes);
	ColorBufferObject2D buf(srgb ? ColorBufferFormat::SRGB8_ALPHA8 : ColorBufferFormat::RGBA8,
		img.width(), img.height(), data);
	buf.setTiling(tile);
	return buf;
}

QImage ColorBufferObject2D::toQImage(int mipLevel)
{
	unsigned char* bytes = readBytes(mipLevel);
	QImage img(bytes, _width, _height, QImage::Format_ARGB32);
	delete bytes;
	return QGLWidget::convertToGLFormat(img); // this function can be applied to GL data to get Qt data again
}

ColorBufferObject3D::ColorBufferObject3D(ColorBufferFormat internalFormat, int width, int height, int depth, std::vector<std::vector<unsigned char>> dataPerMipLevel) :
	_depth(depth), ColorBufferObject2DBase(internalFormat, width, height, dataPerMipLevel)
{
}

void ColorBufferObject3D::setSize(int width, int height, int depth)
{
	assertFatal(_data.size() == 0, "Can not resize a texture that was initialized with data. The data dictates the required resolution.");
	if (width == _width && height == _height && depth == _depth)
		return;
	_width = width;
	_height = height;
	_depth = depth;
	if (_handle != nullptr)
		_sizeChanged();
}

void ColorBufferObject3D::_sizeChanged()
{
	// never called before initialize()

	// reallocate with new size
	glBindTexture(_textureType(), *(GLuint*)_handle);
	for (int mipLevel = 0; mipLevel < _mipLevels; ++mipLevel)
	{
		int factor = 1 << mipLevel;

		// because of glTexImage2D this function must be reimplemented in non-2D texture subclasses
		if (mipLevel >= _data.size())
			gl.glTexImage3D(_textureType(), mipLevel, _internalFormat, _width / factor, _height / factor, _depth / factor, 0, highLevelFormat(format()), formatDataType(format()), nullptr);
		else
			gl.glTexImage3D(_textureType(), mipLevel, _internalFormat, _width / factor, _height / factor, _depth / factor, 0, highLevelFormat(format()), formatDataType(format()), &_data[mipLevel][0]);
		// level 0 has been set, allocate mips before pushing more data
		if (!mipLevel && _data.size() > 1)
			_generateMipMaps();
	}
}

RenderBufferObject::RenderBufferObject(RenderBufferFormat internalFormat, int width, int height) :
	BufferObject2DBase((GLenum)internalFormat, width, height)
{
}

void RenderBufferObject::_sizeChanged()
{
	// reallocate with the new size
	gl.glBindRenderbuffer(GL_RENDERBUFFER, *(GLuint*)_handle);
	gl.glRenderbufferStorage(GL_RENDERBUFFER, _internalFormat, _width, _height);
}

void RenderBufferObject::_initialize()
{
	_handle = new GLint[1];
	gl.glGenRenderbuffers(1, (GLuint*)_handle);
	_sizeChanged();
}

void RenderBufferObject::_uninitialize()
{
	gl.glDeleteRenderbuffers(1, (GLuint*)_handle);
	delete[] _handle;
}

void RenderBufferObject::bind()
{
	gl.glBindRenderbuffer(GL_RENDERBUFFER, *(GLuint*)_handle);
}

ShaderStorageBufferObject::ShaderStorageBufferObject(int size, char* data) :
	_size(size),
	_data(data)
{
}

ShaderStorageBufferObject::~ShaderStorageBufferObject()
{
	delete _data;
}

void ShaderStorageBufferObject::_initialize()
{
	_handle = new GLint[1];
	gl.glGenBuffers(1, (GLuint*)_handle);
	if (_data)
		setData(_size, _data);
	else
		setSize(_size);
}

void ShaderStorageBufferObject::_uninitialize()
{
	gl.glDeleteBuffers(1, (GLuint*)_handle);
	delete[] _handle;
}

void ShaderStorageBufferObject::setSize(int size)
{
	assertFatal(!_data, "Resizing SSBO that is cerated from user data would lose the user data.");
	_size = size;
	delete _data;
	_data = nullptr;
	if (_handle)
	{
		gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, *(GLuint*)_handle);
		gl.glBufferData(GL_SHADER_STORAGE_BUFFER, _size, _data, GL_DYNAMIC_COPY);
		gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}

void ShaderStorageBufferObject::setData(int dataSize, char* data)
{
	delete _data;
	_size = dataSize;
	_data = data;
	if (_handle)
	{
		gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, *(GLuint*)_handle);
		gl.glBufferData(GL_SHADER_STORAGE_BUFFER, _size, _data, GL_DYNAMIC_COPY);
		gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}

void ShaderStorageBufferObject::bind(int index)
{
	gl.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, *(GLuint*)_handle);
}

void ShaderStorageBufferObject::unbindAll()
{
	// TODO: don't we need to unbidn glBindBufferBase here instead?
	gl.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

ColorBufferObjectCube::ColorBufferObjectCube(ColorBufferFormat internalFormat, int size, std::vector<std::vector<unsigned char>> dataPerMipLevelPerFace) :
	ColorBufferObject2DBase(internalFormat, size, size, dataPerMipLevelPerFace)
{
}

void ColorBufferObjectCube::setSize(int width, int height)
{
	assert(_data.size() == 0, "Can not change a texture that was initialized with data. Th data dictates the required resolution.");
	assert(width == height, "Cube maps must be perfectly square.");
	ColorBufferObject2DBase::setSize(width, height);
}

void ColorBufferObjectCube::_sizeChanged()
{
	// reallocate with the new size
	glBindTexture(_textureType(), *(GLuint*)_handle);

	_mipLevels = (int)_data.size() / 6;
	if (_mipLevels == 0) _mipLevels = 1;
	// we must upload level 1 first, then generate mips and then fill in the remaining data
	for (int faceId = 0; faceId < 6; ++faceId)
	{
		unsigned char* data = nullptr;
		if (_data.size())
			data = &_data[faceId * _mipLevels][0];
		glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceId), 0, _internalFormat, _width, _height, 0, highLevelFormat(format()), formatDataType(format()), data);
	}

	// level 0 has been allocated, allocate mips before pushing more data
	_generateMipMaps();

	// I love repetition
	for (int faceId = 0; faceId < 6; ++faceId)
	{
		for (int mipLevel = 1; mipLevel < _mipLevels; ++mipLevel)
		{
			int factor = 1 << mipLevel;
			unsigned char* data = nullptr;
			if (_data.size())
				data = &_data[faceId * _mipLevels + mipLevel][0];
			glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceId), mipLevel, _internalFormat, _width / factor, _height / factor, 0, highLevelFormat(format()), formatDataType(format()), data);
		}
	}
}
