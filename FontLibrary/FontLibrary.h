//=================================================================================
// Name			:	FontLibrary.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CFontLibrary
// this file denotes the basic structure of the font library
// this class uses files produced by the AngelCode - BitmapFontGenerator,
// which produces a config file(*.fnt) and a bitmap file(*.tga) for each font type.
//=================================================================================

#pragma once
#include "Font.h"
#include <Windows.h>
#include <glew.h>
#include "GLShaderProgram.h"
#include <map>

#define MAX_STRING_LENGTH 10922 //because thats limiting our indicies buffer to 16bit: 65535 entries, divided by 6 vertices(2 triangles per quad aka. text char to render), this way we don't have to use 32bit integer

//font type defines, string must match the filename without extension in the "\Fonts" folder!

#define GLFONT_COURIERNEW_MSDF "CourierNew_msdf"
#define GLFONT_COURIERNEW_SDF "CourierNew_sdf"
#define GLFONT_COURIERNEW100 "CourierNew100"
#define GLFONT_ARIAL20 "Arial20"
#define GLFONT_DINNEXTLTPROMED_SDF "DINNextLTProMED_SDF"

//ADD your own here!


//==================================================================================================

class CGLQuad2D
{
public:
	float topLeftX = 0;
	float topLeftY = 0;
	float topRightX = 0;
	float topRightY = 0;
	float bottomRightX = 0;
	float bottomRightY = 0;
	float bottomLeftX = 0;
	float bottomLeftY = 0;

	float textureTopLeftX = 0;
	float textureTopLeftY = 0;
	float textureTopRightX = 0;
	float textureTopRightY = 0;
	float textureBottomRightX = 0;
	float textureBottomRightY = 0;
	float textureBottomLeftX = 0;
	float textureBottomLeftY = 0;
};

class CDrawString
{
public:
	CDrawString(){};

	CDrawString(unsigned int ID, const std::string& font, const std::string& textToDraw, int x, int y, float color[4], float scale)
	{
		m_ID = ID;
		if(textToDraw.size() > MAX_STRING_LENGTH)
			m_text = textToDraw.substr(0, MAX_STRING_LENGTH);
		else
			m_text = textToDraw;
		m_x = x;
		m_y = y;
		m_color[0] = color[0];
		m_color[1] = color[1];
		m_color[2] = color[2];
		m_color[3] = color[3];
		m_scale = scale;
		m_font = font;
	};

	~CDrawString()
	{
		if(m_vertexbuffer)
			glDeleteBuffers(1, &m_vertexbuffer);
		m_vertexbuffer = 0;

		if(m_uvBuffer)
			glDeleteBuffers(1, &m_uvBuffer);
		m_uvBuffer = 0;
	};

	unsigned int	m_ID = 0;
	bool			m_needsChange = true;
	std::string		m_text;
	int				m_x = 0;
	int				m_y = 0;
	float			m_color[4] = {0};
	float			m_scale = 1.0;
	int				m_lineWidth = 0;
	int				m_maxLines = 0;
	uint16_t		m_winW = 1; //window width of application
	uint16_t		m_winH = 1; //window height of application
	GLuint			m_vertexbuffer = 0; //stores verticies positions
	GLuint			m_uvBuffer =  0; //stores texture UVs for each vert
	std::string		m_font = GLFONT_ARIAL20;
	unsigned int	m_numVerticies = 0;
	unsigned int	m_numIndices = 0;
	float			m_bgColor[4] = {0};
	int				m_outline = 0; //can be 0, for no outline 1 or higher
	float			m_outlineColor[4] = {0};
};

class CFontLibrary
{
public:
	//constructor + destructor:
	CFontLibrary(const std::string& folder);
	~CFontLibrary(void);

	//variables:
	std::map<std::string, CGLFont*> m_fontList;
	std::string m_fontFolder;

	//functions:
	bool ParseAllFontsInFolder();
	bool InitGLFonts(bool compressNonSDFTextures = false);
	void DrawString(const std::string& textToDraw, int x, int y, float color[4], const std::string& font, unsigned int winW, unsigned int winH, float scale = 1.0f, int outline = 0, float outlineColor[4] = {0}, float bgcolor[4] = {0}); //bgColor is only taken into account when using multi channel SDFs
	void DrawString(unsigned int ID, const std::string& textToDraw, int x, int y, float color[4], const std::string& font, unsigned int winW, unsigned int winH, float scale = 1.0f, int outline = 0, float outlineColor[4] = {0}, float bgcolor[4] = {0}); //bgColor is only taken into account when using multi channel SDFs
	void DrawStringWithLineBreaks(const std::string& textToDraw, int x, int y, float color[4], const std::string& font, unsigned int winW, unsigned int winH, float scale, int lineWidth, int maxLines, int outline = 0, float outlineColor[4] = {0});
	void DrawStringWithLineBreaks(unsigned int ID, const std::string& textToDraw, int x, int y, float color[4], const std::string& font, unsigned int winW, unsigned int winH, float scale, int lineWidth, int maxLines, int outline = 0, float outlineColor[4] = {0});
	unsigned int GetNewDrawStringID();
	float GetWidthOfString(const std::string& textToDraw, const std::string& font, float scale = 1.0f, bool ignoreKerning = true);
	unsigned int GetLineHeight(const std::string& font);

private:
	//functions:
	float AdjustForKerningPairs(const std::string& font, const std::string& textToDraw, int first, char second, float scale);
	CGLFont* ParseFont(const std::string& fileName);
	CGLQuad2D* TextToQuadList(const std::string& font, const std::string& textToDraw, int x, int y, float scale);
	void CFontLibrary::DrawTriangles(const std::string& font, float color[4], CDrawString* stringObject, int outline = 0, float outlineColor[4] = {0}, float bgColor[4] = {0});
	CGLFont* GetFontPointer(const std::string& fontName);
	int GetTextChar(const std::string& textToDraw, int pos);
	unsigned int GetWidthOfChar(char ch, const std::string& font);
	void PopulateVertexBuffers(CDrawString* stringObject);

	//variables:
	CGLShaderProgram* m_sdfShaderProgram = nullptr;
	CGLShaderProgram* m_defaultShaderProgram = nullptr;
	std::map<unsigned int, CDrawString*> m_glStringList;
	unsigned int m_IDCounter = 1;
	GLuint m_vertexArrayID = 0; // Our Vertex Array Object, once for all!
};