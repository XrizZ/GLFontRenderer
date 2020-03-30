//=================================================================================
// Name			:	glutcallbacks.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Example Implementation for FontLibrary - Header File for
//					Glutcallbacks.
//=================================================================================

void Init(void);
void RegisterCallbacks(void);
void Display(void);
void Reshape(int width, int height);
void Mouse(int btn, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void MouseMotion(int x, int y);
void MouseMenu(int id);