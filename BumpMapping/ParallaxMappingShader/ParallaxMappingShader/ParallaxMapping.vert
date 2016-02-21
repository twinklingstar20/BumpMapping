#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 Vertex;
layout(location = 1) in vec2 VertexUV;
layout(location = 2) in vec3 NormalModelSpace;
layout(location = 3) in vec3 TangentModelSpace;
layout(location = 4) in vec3 BitangentModelspace;

// Output data ; will be interpolated for each fragment.
out vec2 TexVertUV;

out vec3 EyeTangSpace;
out vec3 LightTangSpace;

// Values that stay constant for the whole mesh.
uniform mat4 ModelViewProj;
uniform mat4 View;
uniform mat4 Model;
uniform mat3 MV3x3;
uniform vec3 LightPos;
uniform int gSwitch;

void main()
{
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  ModelViewProj * vec4(Vertex, 1);
	// the View matrix is identity here.
	// Position of the vertex, in worldspace : Model * position
	vec3 posWorldSpace = (Model * vec4(Vertex,1)).xyz;

	// Vector that goes from the vertex to the light.
	vec3 lightPosWorldSpace = (View * vec4(LightPos,1)).xyz;
	vec3 lightDirWorldSpace = normalize(lightPosWorldSpace - posWorldSpace);

	vec3 camPosWorldSpace = vec3(0,0,0);
	vec3 camDirCamSpace = normalize(camPosWorldSpace - posWorldSpace);
	
	// UV of the vertex. No special space for this one.
	TexVertUV = VertexUV;
	
	// model to camera = ModelView
	vec3 TangentCamSpace = MV3x3 * TangentModelSpace;
	vec3 BitangentCamSpace = MV3x3 * BitangentModelspace;
	vec3 NormalCamSpace = MV3x3 * NormalModelSpace;

	// You can use dot products instead of building this matrix and transposing it. See References for details.
	mat3 TBN = transpose(mat3(TangentCamSpace,BitangentCamSpace,NormalCamSpace)); 

	LightTangSpace = TBN * lightDirWorldSpace;
	EyeTangSpace =  TBN * camDirCamSpace;
}

