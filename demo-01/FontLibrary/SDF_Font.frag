R"(
	varying vec4 v_Color;
	uniform sampler2D u_FontTexutre;
	const float smoothing = 1.0/16.0;

	void main(void)
	{
		vec4 col = v_Color;
		float mask = texture2D(u_FontTexutre, gl_TexCoord[0].xy).a;
		float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, mask);
		gl_FragColor = vec4(col.rgb, col.a * alpha);
	}
)";