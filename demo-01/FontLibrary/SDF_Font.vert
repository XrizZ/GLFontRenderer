R"(
	varying vec4 v_Color;

	void main(void)
	{
		gl_TexCoord[0] = gl_MultiTexCoord0;
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		v_Color = gl_Color;
	}
)";