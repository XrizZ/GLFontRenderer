#include "GLShaderProgram.h"
#include <cassert>
#include <sstream>
#include <fstream>

CGLShaderProgram::CGLShaderProgram(const CGLShaderProgram & glShaderProgram)
{
	m_vertexShader = glShaderProgram.m_vertexShader;
	m_fragmentShader = glShaderProgram.m_fragmentShader;
	m_vertexSource = glShaderProgram.m_vertexSource;
	m_fragmentSource = glShaderProgram.m_fragmentSource;
	m_shaderProgram = glShaderProgram.m_shaderProgram;
}

CGLShaderProgram::~CGLShaderProgram()
{
	CleanUp();
}

void CGLShaderProgram::CleanUp()
{
	// Detaching the shaders from the program
	if (m_shaderProgram > 0 && m_vertexShader > 0)
		glDetachShader(m_shaderProgram, m_vertexShader);
	
	if (m_shaderProgram > 0 && m_fragmentShader > 0)
		glDetachShader(m_shaderProgram, m_fragmentShader);

	//deleteing shaders and program
	if(m_vertexShader)
		glDeleteShader(m_vertexShader);

	if(m_fragmentShader)
		glDeleteShader(m_fragmentShader);

	if(m_shaderProgram)
		glDeleteProgram(m_shaderProgram);

	m_vertexShader = 0;
	m_fragmentShader = 0;
	m_shaderProgram = 0;
}

GLuint CGLShaderProgram::CompileShader(GLenum aShaderType, const char* aShaderSource)
{
	GLuint shaderHandle = glCreateShader(aShaderType);

	const char* shaderName[] = { aShaderSource };

	glShaderSource(shaderHandle, 1, shaderName, 0);

	glCompileShader(shaderHandle);

	GLenum errorCode = glGetError();

	int32_t compiled = 0;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);

	if(!compiled)
	{
		GLint blen = 0;
		GLsizei slen = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &blen);

		if(blen > 1)
		{
			char* compilerLog = new char[blen + 1];
			glGetShaderInfoLog(shaderHandle, blen, &slen, compilerLog);
			std::cout << "GLSL Compiler Error: " << compilerLog;
			delete []compilerLog;
		}

		glDeleteShader(shaderHandle);
		return 0;
	}

	return shaderHandle;
}

bool CGLShaderProgram::InitFromString(const std::string& vertexShaderSourceCode, const std::string& fragmentShaderSourceCode)
{
	assert(!vertexShaderSourceCode.empty() && !fragmentShaderSourceCode.empty());
	assert(m_vertexSource.empty() && m_fragmentSource.empty());
	assert(m_vertexShader == 0 && m_fragmentShader == 0 && m_shaderProgram == 0);

	m_vertexSource = vertexShaderSourceCode;
	m_fragmentSource = fragmentShaderSourceCode;

	m_vertexShader = CompileShader(GL_VERTEX_SHADER, m_vertexSource.c_str());
	m_fragmentShader = CompileShader(GL_FRAGMENT_SHADER, m_fragmentSource.c_str());

	if (m_vertexShader == 0 && m_fragmentShader == 0)
	{
		CleanUp();
		return false;
	}

	m_shaderProgram = glCreateProgram();

	glAttachShader(m_shaderProgram, m_vertexShader);
	glAttachShader(m_shaderProgram, m_fragmentShader);

	glLinkProgram(m_shaderProgram);

	CheckForErrors();

	int32_t linked = 0;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		int32_t infoLength;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);

		if(infoLength)
		{
			char* infoBuffer = new char[infoLength + 2];
			glGetProgramInfoLog(m_shaderProgram, infoLength + 1, 0, infoBuffer);

			int8_t n = 10; //number of first few characters of shadercode to show in error message
			std::cout << "GLSL Linker Error in " << vertexShaderSourceCode.substr(0,n).c_str() << " " << fragmentShaderSourceCode.substr(0, n).c_str() << ": " << infoBuffer;
			delete []infoBuffer;
		}

		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;

		return false;
	}
	
	return true;
}

bool CGLShaderProgram::InitFromFile(const std::string& aVertexFileName, const std::string& aFragmentFileName)
{
	assert(!aVertexFileName.empty() && !aFragmentFileName.empty());
	assert(m_vertexSource.empty() && m_fragmentSource.empty());
	assert(m_vertexShader == 0 && m_fragmentShader == 0 && m_shaderProgram == 0);

	m_vertexSource = LoadCompleteFile(aVertexFileName.c_str());
	m_fragmentSource = LoadCompleteFile(aFragmentFileName.c_str());

	m_vertexShader = CompileShader(GL_VERTEX_SHADER, m_vertexSource.c_str());
	m_fragmentShader = CompileShader(GL_FRAGMENT_SHADER, m_fragmentSource.c_str());

	if (m_vertexShader == 0 && m_fragmentShader == 0)
	{
		CleanUp();
		return false;
	}

	m_shaderProgram = glCreateProgram();

	glAttachShader(m_shaderProgram, m_vertexShader);
	glAttachShader(m_shaderProgram, m_fragmentShader);

	glLinkProgram(m_shaderProgram);

	CheckForErrors();

	int32_t linked = 0;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		int32_t infoLength;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);

		if(infoLength)
		{
			char* infoBuffer = new char[infoLength + 1];
			glGetProgramInfoLog(m_shaderProgram, infoLength, 0, infoBuffer);
			std::cout << "GLSL Linker Error in " << aVertexFileName << " " << aFragmentFileName << ": " << infoBuffer;
			delete []infoBuffer;
		}

		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;

		return false;
	}
	
	return true;
}

bool CGLShaderProgram::CheckForErrors(std::string aFile, int32_t aLine)
{
	GLenum errorID = GL_NO_ERROR;
	std::string error;
	uint32_t errorCount = 0;

	while ((errorID = glGetError()) != GL_NO_ERROR)
	{
		errorCount++;
		switch (errorID)
		{
			case GL_INVALID_ENUM:
				error = "GL_INVALID_ENUM";
				break;

			case GL_INVALID_VALUE:
				error = "GL_INVALID_VALUE";
				break;

			case GL_INVALID_OPERATION:
				error = "GL_INVALID_OPERATION";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;

			case GL_OUT_OF_MEMORY:
				error = "GL_OUT_OF_MEMORY";
				break;

			default:
				error = "Unknown GL error Type";
				break;
		}

		std::cout << "GLSL Error -> " << errorID << " - " << error << " - " << errorCount << " - " << aFile << "(" << aLine <<")\n";
		assert(false);
	}

	return errorCount > 0 ? true : false;
}

std::string CGLShaderProgram::LoadCompleteFile(std::string aFileName)
{
	std::string buffer;

	std::ifstream file;
	file.open(aFileName, std::ios::in | std::ios::binary);

	if(!file.fail())
	{
		while(!file.eof())
		{
			char currChar;
			file.get(currChar);
			buffer.push_back(currChar);
		}
		file.close();
	}

	return buffer;
}