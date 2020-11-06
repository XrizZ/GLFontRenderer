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
	GLfloat topLeftX = 0;
	GLfloat topLeftY = 0;
	GLfloat topRightX = 0;
	GLfloat topRightY = 0;
	GLfloat bottomRightX = 0;
	GLfloat bottomRightY = 0;
	GLfloat bottomLeftX = 0;
	GLfloat bottomLeftY = 0;

	GLfloat textureTopLeftX = 0;
	GLfloat textureTopLeftY = 0;
	GLfloat textureTopRightX = 0;
	GLfloat textureTopRightY = 0;
	GLfloat textureBottomRightX = 0;
	GLfloat textureBottomRightY = 0;
	GLfloat textureBottomLeftX = 0;
	GLfloat textureBottomLeftY = 0;
};

class CDrawString
{
public:
	CDrawString(){};

	CDrawString(uint16_t ID, const std::string& font, const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], float scale)
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

	uint16_t		m_ID = 0;
	bool			m_needsChange = true;
	std::string		m_text;
	int16_t			m_x = 0;
	int16_t			m_y = 0;
	GLfloat			m_color[4] = {0};
	float			m_scale = 1.0f;
	uint16_t		m_lineWidth = 0;
	uint16_t		m_maxLines = 0;
	uint16_t		m_winW = 1; //window width of application
	uint16_t		m_winH = 1; //window height of application
	GLuint			m_vertexbuffer = 0; //stores verticies positions
	GLuint			m_uvBuffer =  0; //stores texture UVs for each vert
	std::string		m_font = GLFONT_ARIAL20;
	uint16_t		m_numVerticies = 0;
	uint16_t		m_numIndices = 0;
	GLfloat			m_bgColor[4] = {0.0f};
	uint16_t		m_outline = 0; //can be 0, for no outline 1 or higher
	GLfloat			m_outlineColor[4] = {0.0f};
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
	void DrawString(const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, GLfloat scale = 1.0f, uint16_t outline = 0, GLfloat outlineColor[4] = {0}, GLfloat bgcolor[4] = {0}); //bgColor is only taken into account when using multi channel SDFs
	void DrawString(uint16_t ID, const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale = 1.0f, uint16_t outline = 0, GLfloat outlineColor[4] = {0}, GLfloat bgcolor[4] = {0}); //bgColor is only taken into account when using multi channel SDFs
	void DrawStringWithLineBreaks(const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale, uint16_t lineWidth, uint16_t maxLines, uint16_t outline = 0, GLfloat outlineColor[4] = {0});
	void DrawStringWithLineBreaks(uint16_t ID, const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW,uint16_t winH, float scale, uint16_t lineWidth, uint16_t maxLines, uint16_t outline = 0, GLfloat outlineColor[4] = {0});
	uint16_t GetNewDrawStringID();
	float GetWidthOfString(const std::string& textToDraw, const std::string& font, float scale = 1.0f, bool ignoreKerning = true);
	uint16_t GetLineHeight(const std::string& font);

private:
	//functions:
	float AdjustForKerningPairs(const std::string& font, const std::string& textToDraw, uint16_t first, char second, float scale);
	CGLFont* ParseFont(const std::string& fileName);
	CGLQuad2D* TextToQuadList(const std::string& font, const std::string& textToDraw, int16_t x, int16_t y, float scale);
	void CFontLibrary::DrawTriangles(const std::string& font, GLfloat color[4], CDrawString* stringObject, uint16_t outline = 0, GLfloat outlineColor[4] = {0}, GLfloat bgColor[4] = {0});
	CGLFont* GetFontPointer(const std::string& fontName);
	uint32_t GetTextChar(const std::string& textToDraw, uint32_t pos);
	uint16_t GetWidthOfChar(char ch, const std::string& font);
	void PopulateVertexBuffers(CDrawString* stringObject);

	//variables:
	CGLShaderProgram* m_sdfShaderProgram = nullptr;
	CGLShaderProgram* m_defaultShaderProgram = nullptr;
	std::map<uint16_t, CDrawString*> m_glStringList;
	uint16_t m_IDCounter = 1;
	GLuint m_vertexArrayID = 0; // Our Vertex Array Object, once for all!
};