R"(
	precision highp float;
	uniform sampler2D u_FontTexutre;
	uniform int sdfType;
	uniform float pxRange;
	uniform vec4 fgColor;
	uniform vec4 bgColor;

	const float smoothing = 1.0/16.0;

	float median(float r, float g, float b)
	{
		return max(min(r, g), min(max(r, g), b));
	}

	void main(void)
	{
		if(sdfType == 1)
		{
			vec4 col = fgColor;
			float mask = texture2D(u_FontTexutre, gl_TexCoord[0].xy).a;
			float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, mask);
			gl_FragColor = vec4(col.rgb, col.a * alpha);
		}
		else
		{
			vec2 msdfUnit = pxRange/vec2(textureSize(u_FontTexutre, 0));
			vec3 sample = texture2D(u_FontTexutre, gl_TexCoord[0].xy).rgb;
			float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
			sigDist *= dot(msdfUnit, 0.5/fwidth(gl_TexCoord[0].xy));
			float opacity = clamp(sigDist + 0.5, 0.0, 1.0);
			gl_FragColor = mix(bgColor, fgColor, opacity);
		}
	}
)";