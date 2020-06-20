R"(
	#version 330 core
	precision highp float;
	in vec2 uv;

	uniform sampler2D u_FontTexture;
	uniform int sdfType;
	uniform float pxRange;
	uniform vec4 fgColor;
	uniform vec4 bgColor;
	uniform vec4 outlineColor;
	uniform int outline;

	out vec4 FragColor;

	const float smoothing = 1.0/16.0;

	float median(float r, float g, float b)
	{
		return max(min(r, g), min(max(r, g), b));
	}

	void main(void)
	{
		if(sdfType == 1) //single channel sdf
		{
			vec4 col = fgColor;
			float mask = texture(u_FontTexture, uv).a;
			float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, mask);

			if(outline > 0 && mask > (0.5 - outline*0.075) && mask <= 0.51)
				FragColor = outlineColor;
			else
				FragColor = vec4(col.rgb, col.a * alpha);
		}
		else //multi channel sdf
		{
			vec2 msdfUnit = pxRange/vec2(textureSize(u_FontTexture, 0));
			vec3 sample = texture(u_FontTexture, uv).rgb;
			float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
			sigDist *= dot(msdfUnit, 0.5/fwidth(uv));
			float opacity = clamp(sigDist + 0.5, 0.0, 1.0);

			if(outline > 0 && sigDist > (-0.5 - outline*0.5) && sigDist <= 1.0)
				FragColor = outlineColor;
			else
				FragColor = mix(bgColor, fgColor, opacity);
		}
	}
)";