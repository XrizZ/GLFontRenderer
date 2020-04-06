#include "stdafx.h"
#include "GLShaderProgram.h"
#include <cassert>

CGLShaderProgram::CGLShaderProgram()
{
	m_vertexShader = 0;
	m_fragmentShader = 0;
	m_shaderProgram = 0;
}

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
	{
		glDetachShader(m_shaderProgram, m_vertexShader);
	}
	
	if (m_shaderProgram > 0 && m_fragmentShader > 0)
	{
		glDetachShader(m_shaderProgram, m_fragmentShader);
	}

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

	int compiled = 0;
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

			CString infoText;
			infoText.Format("GLSL Compiler Error: %s",  CString(compilerLog));
			TRACE(infoText);
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

	int linked = 0;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		int infoLength;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);

		if(infoLength)
		{
			char* infoBuffer = new char[infoLength + 2];
			glGetProgramInfoLog(m_shaderProgram, infoLength + 1, 0, infoBuffer);

			int n = 10; //number of first few characters of shadercode to show in error message
			CString infoText;
			infoText.Format("GLSL Linker Error in %s, %s: %s", vertexShaderSourceCode.substr(0,n).c_str(), fragmentShaderSourceCode.substr(0, n).c_str(), CString(infoBuffer));
			TRACE(infoText);
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

	int linked = 0;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		int infoLength;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLength);

		if(infoLength)
		{
			char* infoBuffer = new char[infoLength + 1];
			glGetProgramInfoLog(m_shaderProgram, infoLength, 0, infoBuffer);

			CString infoText;
			infoText.Format("GLSL Linker Error in %s, %s: %s", aVertexFileName, aFragmentFileName, infoBuffer);
			TRACE(infoText);
			delete []infoBuffer;
		}

		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;

		return false;
	}
	
	return true;
}

bool CGLShaderProgram::CheckForErrors(CString aFile, int aLine)
{
	GLenum errorID = GL_NO_ERROR;
	CString error;
	int errorCount = 0;

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

		CString output;
		output.Format("GLSL Error -> %d - %s - %d - %s(%d)\n", errorID, error, errorCount, aFile, aLine);
		TRACE(output);
		assert(false);
	}

	return errorCount > 0 ? true : false;
}

CString CGLShaderProgram::LoadCompleteFile(CString aFileName)
{
	FILE* pShaderFile;
	errno_t err = fopen_s(&pShaderFile, aFileName, "rb");

	CString buffer;

	if (pShaderFile)
	{
		fseek(pShaderFile, 0, SEEK_END);
		long size = ftell(pShaderFile);
		rewind(pShaderFile);

		TCHAR *pStr = buffer.GetBufferSetLength(size);
		fread(pStr, sizeof(TCHAR), size, pShaderFile);
		buffer.ReleaseBuffer();

		fclose(pShaderFile);

		return buffer;
	}

	return buffer;
}
