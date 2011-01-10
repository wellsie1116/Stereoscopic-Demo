#extension GL_EXT_gpu_shader4 : enable

varying vec3 position;

varying vec4 gl_Color;

varying float fogFactor;

void main()
{
	vec3 center = vec3(0.0, 0.0, 0.0);
	float dist = distance(center, position);
	if (dist > 2.0)
		discard;

	vec4 targetColor;
	if (int(dist * 3.5) % 2 == 1)
		targetColor = vec4(1.0, 1.0, 1.0, 1.0);
	else
		targetColor = gl_Color;

	gl_FragColor = mix(gl_Fog.color, targetColor, fogFactor);
}
