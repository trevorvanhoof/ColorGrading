#pragma once

#include "gl.h"
#include "buffers.h"
#include <map>

typedef QString FilePath;

enum class ProgramStage : GLenum
{
	frag = GL_FRAGMENT_SHADER,
	vert = GL_VERTEX_SHADER,
	geometry = GL_GEOMETRY_SHADER,
};

class Shader
{
protected:
	FilePath _filePath;
	ProgramStage _stage;

public:
	Shader(FilePath filePath, ProgramStage stage);
	inline FilePath filePath() const { return _filePath; }
	inline ProgramStage stage() const { return _stage; }
};

class Program
{
protected:
	std::vector<Shader> _shaders;

public:
	Program();
	Program(Shader& shader);
	Program(std::vector<Shader> shaders);
	void bind() const;

	// uniform setters
	void set(char* key, float value);
	void set(char* key, float x, float y);
	void set(char* key, float x, float y, float z);
	void set(char* key, float x, float y, float z, float w);
	void set(char* key, int value);
	void set(char* key, int x, int y);
	void set(char* key, int x, int y, int z);
	void set(char* key, int x, int y, int z, int w);
	void set(char* key, unsigned int value);
	void set(char* key, unsigned int x, unsigned int y);
	void set(char* key, unsigned int x, unsigned int y, unsigned int z);
	void set(char* key, unsigned int x, unsigned int y, unsigned int z, unsigned int w);
	void set(char* key, int location, ColorBufferObject2DBase& texture);
	void set(char* key, QVector2D vec);
	void set(char* key, QVector3D vec);
	void set(char* key, QVector4D vec);
	void set(char* key, QMatrix2x2 mat);
	void set(char* key, QMatrix3x3 mat);
	void set(char* key, QMatrix4x4 mat);
	void set(char* key, std::vector<float> value);
	void set(char* key, std::vector<int> value);
	void set(char* key, std::vector<QVector2D> value);
	void set(char* key, std::vector<QVector3D> value);
	void set(char* key, std::vector<QVector4D> value);
	void set(char* key, std::vector<QMatrix2x2> value);
	void set(char* key, std::vector<QMatrix3x3> value);
	void set(char* key, std::vector<QMatrix4x4> value);
};
