#pragma once
#include <glew.h>
#include <iostream>

class CGLShaderProgram
{
public:
	CGLShaderProgram(){};
	CGLShaderProgram(const CGLShaderProgram& glShaderProgram);
	~CGLShaderProgram();

	// FUNCTIONS TO CREATE THE SHADER PROGRAM
	bool		InitFromFile(const std::string& aVertexShaderFileName, const std::string& aFragmentShaderFileName);
	bool		InitFromString(const std::string& vertexShaderSourceCode, const std::string& fragmentShaderSourceCode);
	GLuint		GetProgramID(){return m_shaderProgram;};

protected:
	// PROTECTED VARIABLES
	std::string		m_vertexSource;
	std::string		m_fragmentSource;

	GLuint			m_vertexShader = 0;
	GLuint			m_fragmentShader = 0;
	GLuint			m_shaderProgram = 0;

private:

	// For cleanup
	void CleanUp();

	// HELPER FUNTIONS FOR CREATING SHADER PROGRAM
	GLuint		CompileShader(GLenum aShaderType, const char* aShaderSource);
	bool		CheckForErrors(std::string aFile = __FILE__, int32_t aLine = __LINE__);
	std::string		LoadCompleteFile(std::string aFileName);
};