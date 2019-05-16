#include "materials.h"
#include "alerts.h"

QString readWithIncludes(FilePath filePath, std::vector<FilePath> readFiles)
{
	QFile fh(filePath);
	if (!fh.open(QFile::ReadOnly | QFile::Text))
	{
		CONVERT_QSTRING(filePath, text);
		warning("Could not read file '%s'", text);
		return "";
	}
	QTextStream s(&fh);
	return s.readAll();
}
FilePath sanitizePath(FilePath filePath) { return QFileInfo(filePath).absoluteFilePath().toLower(); }

std::map<QString, GLuint> shaderCache;
std::map<QString, GLuint> programCache;
std::map<QString, QString> fileKeyAssociation;
std::map<QString, std::vector<QString>> shaderSourceFiles;

class ShaderWatcher : public QFileSystemWatcher
{
protected:
	void onFileChanged(QString changedPath)
	{
		changedPath = sanitizePath(changedPath);
		bool ok = false;
		while (!ok)
		{
			addPath(changedPath);
			for (auto path : files())
			{
				if (sanitizePath(path) == changedPath)
				{
					ok = true;
					break;
				}
			}
			// todo: sleep ~ 5 ms
		}
		for (auto key : fileKeyAssociation[changedPath])
		{
			if (shaderCache.count(key))
			{
				gl.glDeleteShader(shaderCache[key]);
				shaderCache.erase(key);
			}
			if (programCache.count(key))
			{
				gl.glDeleteProgram(programCache[key]);
				programCache.erase(key);
			}
		}
	}

public:
	ShaderWatcher()
	{
		connect(this, &QFileSystemWatcher::fileChanged, this, &ShaderWatcher::onFileChanged);
	}
};

const int INFO_LOG_BUFFER_SIZE = 1024 * 1024; // 1 MB a bit overkill?
char* INFO_LOG_BUFFER = nullptr;

GLuint compileShader(QString source, ProgramStage stage)
{
	GLuint shader = gl.glCreateShader((GLenum)stage);
	CONVERT_QSTRING(source, text);
	gl.glShaderSource(shader, 1, &text, nullptr);
	gl.glCompileShader(shader);
	int status;
	gl.glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		if (!INFO_LOG_BUFFER)
			INFO_LOG_BUFFER = new char[INFO_LOG_BUFFER_SIZE];
		int strLen;
		gl.glGetShaderInfoLog(shader, INFO_LOG_BUFFER_SIZE, &strLen, INFO_LOG_BUFFER);
		warning(INFO_LOG_BUFFER);
	}
	return shader;
}

QString shaderKey(const Shader& shader)
{
	char key[sizeof(GLenum)];
	GLenum stage = (GLenum)shader.stage();
	CopyMemory(key, &stage, sizeof(GLenum));
	return shader.filePath() + key;
}

GLuint fetchShader(const Shader& shader)
{
	QString filePath = sanitizePath(shader.filePath());
	QString key = shaderKey(shader);
	if (!shaderCache.count(key))
	{
		std::vector<FilePath> readFiles;
		GLuint shaderi = compileShader(readWithIncludes(filePath, readFiles), shader.stage());
		shaderCache[key] = shaderi;
		for (auto assocFilePath : readFiles)
		{
			fileKeyAssociation[assocFilePath].push_back(key);
		}
		shaderSourceFiles[key] = readFiles;
	}
	return shaderCache[key];
}

GLuint compileProgram(const std::vector<Shader>& shaders)
{
	GLuint program = gl.glCreateProgram();
	for (auto shader : shaders)
		gl.glAttachShader(program, fetchShader(shader));

	gl.glLinkProgram(program);
	
	int status;
	
	gl.glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (!status)
	{
		if (!INFO_LOG_BUFFER)
			INFO_LOG_BUFFER = new char[INFO_LOG_BUFFER_SIZE];
		int strLen;
		gl.glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, &strLen, INFO_LOG_BUFFER);
		warning(INFO_LOG_BUFFER);
	}
	
	gl.glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		if (!INFO_LOG_BUFFER)
			INFO_LOG_BUFFER = new char[INFO_LOG_BUFFER_SIZE];
		int strLen;
		gl.glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, &strLen, INFO_LOG_BUFFER);
		warning(INFO_LOG_BUFFER);
	}
	
	return program;
}

GLuint fetchProgram(const std::vector<Shader>& shaders)
{
	QString key;
	for (auto shader : shaders)
	{
		key += shaderKey(shader);
	}
	if (!programCache.count(key))
	{
		GLuint program = compileProgram(shaders);
		programCache[key] = program;
		for (auto shader : shaders)
		{
			for (auto assocFilePath : shaderSourceFiles[shaderKey(shader)])
			{
				fileKeyAssociation[assocFilePath].push_back(key);
			}
		}
	}
	return programCache[key];
}

Shader::Shader(FilePath filePath, ProgramStage stage) :
	_filePath(filePath),
	_stage(stage)
{
}

Program::Program()
{
}

Program::Program(Shader& shader)
{
	_shaders.push_back(shader);
}

Program::Program(std::vector<Shader> shaders) :
	_shaders(shaders)
{
}

void Program::bind() const
{
	gl.glUseProgram(fetchProgram(_shaders));
}

#define UNIFORM_LOC GLint loc = gl.glGetUniformLocation(fetchProgram(_shaders), key); \
if (loc == -1) \
{ \
	infod("Skipping uniform '%s'. Not found.", key); \
	return; \
}

void Program::set(char* key, float value) { UNIFORM_LOC; gl.glUniform1f(loc, value); }
void Program::set(char* key, float x, float y) { UNIFORM_LOC; gl.glUniform2f(loc, x, y); }
void Program::set(char* key, float x, float y, float z) { UNIFORM_LOC; gl.glUniform3f(loc, x, y, z); }
void Program::set(char* key, float x, float y, float z, float w) { UNIFORM_LOC; gl.glUniform4f(loc, x, y, z, w); }

void Program::set(char* key, int value) { UNIFORM_LOC; gl.glUniform1i(loc, value); }
void Program::set(char* key, int x, int y) { UNIFORM_LOC; gl.glUniform2i(loc, x, y); }
void Program::set(char* key, int x, int y, int z) { UNIFORM_LOC; gl.glUniform3i(loc, x, y, z); }
void Program::set(char* key, int x, int y, int z, int w) { UNIFORM_LOC; gl.glUniform4i(loc, x, y, z, w); }

void Program::set(char* key, unsigned int value) { UNIFORM_LOC; gl.glUniform1ui(loc, value); }
void Program::set(char* key, unsigned int x, unsigned int y) { UNIFORM_LOC; gl.glUniform2ui(loc, x, y); }
void Program::set(char* key, unsigned int x, unsigned int y, unsigned int z) { UNIFORM_LOC; gl.glUniform3ui(loc, x, y, z); }
void Program::set(char* key, unsigned int x, unsigned int y, unsigned int z, unsigned int w) { UNIFORM_LOC; gl.glUniform4ui(loc, x, y, z, w); }

void Program::set(char* key, int location, ColorBufferObject2DBase& texture)
{
	gl.glActiveTexture(GL_TEXTURE0 + location);
	texture.bind();
	UNIFORM_LOC;
	gl.glUniform1i(loc, location);
}
void Program::set(char* key, QVector2D vec) { UNIFORM_LOC; gl.glUniform2f(loc, vec.x(), vec.y()); }
void Program::set(char* key, QVector3D vec) { UNIFORM_LOC; gl.glUniform3f(loc, vec.x(), vec.y(), vec.z()); }
void Program::set(char* key, QVector4D vec) { UNIFORM_LOC; gl.glUniform4f(loc, vec.x(), vec.y(), vec.z(), vec.w()); }
void Program::set(char* key, QMatrix2x2 mat) { UNIFORM_LOC; gl.glUniformMatrix2fv(loc, 1, false, mat.constData()); }
void Program::set(char* key, QMatrix3x3 mat) { UNIFORM_LOC; gl.glUniformMatrix3fv(loc, 1, false, mat.constData()); }
void Program::set(char* key, QMatrix4x4 mat) { UNIFORM_LOC; gl.glUniformMatrix4fv(loc, 1, false, mat.constData()); }
void Program::set(char* key, std::vector<float> value) { UNIFORM_LOC; gl.glUniform1fv(loc, (GLsizei)value.size(), &value[0]); }
void Program::set(char* key, std::vector<int> value) { UNIFORM_LOC; gl.glUniform1iv(loc, (GLsizei)value.size(), &value[0]); }
void Program::set(char* key, std::vector<QVector2D> value) { UNIFORM_LOC; gl.glUniform2fv(loc, (GLsizei)value.size(), (float*)&value[0]); }
void Program::set(char* key, std::vector<QVector3D> value) { UNIFORM_LOC; gl.glUniform3fv(loc, (GLsizei)value.size(), (float*)&value[0]); }
void Program::set(char* key, std::vector<QVector4D> value) { UNIFORM_LOC; gl.glUniform4fv(loc, (GLsizei)value.size(), (float*)&value[0]); }
void Program::set(char* key, std::vector<QMatrix2x2> value) { UNIFORM_LOC; gl.glUniformMatrix2fv(loc, (GLsizei)value.size(), false, value[0].constData()); }
void Program::set(char* key, std::vector<QMatrix3x3> value) { UNIFORM_LOC; gl.glUniformMatrix3fv(loc, (GLsizei)value.size(), false, value[0].constData()); }
void Program::set(char* key, std::vector<QMatrix4x4> value) { UNIFORM_LOC; gl.glUniformMatrix4fv(loc, (GLsizei)value.size(), false, value[0].constData()); }

#undef UNIFORM_LOC