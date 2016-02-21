#version 330 core

// Interpolated values from the vertex shaders
in vec2 TexVertUV;
in vec3 LightDirTangSpace;
in vec3 EyeDirTangSpace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D DiffTexSampler;
uniform sampler2D NormTexSampler;
uniform vec3 LightPos;

void main()
{
	// Light emission properties
	// You probably want to put them as uniforms
	vec3 lightColor = vec3(1,1,1);
	float LightPower = 3.0;
	
	// Material properties
	vec3 d = texture2D(DiffTexSampler, TexVertUV).rgb;

	// Normal of the computed fragment, in camera space
	vec3 n = normalize(texture2D(NormTexSampler, vec2(TexVertUV.x, TexVertUV.y)).rgb * 2.0 - 1.0);
	
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(LightDirTangSpace);
	
	vec3 v = normalize(EyeDirTangSpace);

	float power = 0.3;
	// ambient lighting
	float iamb = 0.1;

	// diffuse lighting
	float idiff = clamp(dot(n, l), 0, 1);

	float ispec = clamp(dot(v + l, n), 0, 1) * power;
	
	color = d * (iamb + idiff + ispec);
}