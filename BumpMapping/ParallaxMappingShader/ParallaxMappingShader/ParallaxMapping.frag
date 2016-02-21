#version 330 core

// Interpolated values from the vertex shaders
in vec2 TexVertUV;
in vec3 EyeTangSpace;
in vec3 LightTangSpace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D DiffTexSampler;
uniform sampler2D NormTexSampler;
uniform vec3 LightPos;
uniform int gSwitch;

float gHeightScale = 0.1;

vec2 pallaxWithOffsetLimit(in vec3 v, in vec2 t)
{
	float height =  texture(NormTexSampler, t).a;    
	vec2 offset = v.xy / v.z * height * gHeightScale;
	return t - offset;
}

vec2 steepPallaxMapping(in vec3 v, in vec2 t)
{
	// determine number of layers from angle between V and N
	const float minLayers = 5;
	const float maxLayers = 15;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), v)));

	// height of each layer
	float layerHeight = 1.0 / numLayers;
	// depth of current layer
	float currentLayerHeight = 0;
	// shift of texture coordinates for each iteration
	vec2 dtex = gHeightScale * v.xy / v.z / numLayers;

 	// current texture coordinates
	vec2 currentTextureCoords = t;

	// get first depth from heightmap
 	float heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;

	// while point is above surface
	while(heightFromTexture > currentLayerHeight) 
	{
		// to the next layer
		currentLayerHeight += layerHeight;
		// shift texture coordinates along vector V
		currentTextureCoords -= dtex;
		// get new depth from heightmap
		heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;
   }

   return currentTextureCoords;
}

vec2 reliefPallaxMapping(in vec3 v, in vec2 t)
{
	// determine required number of layers
	const float minLayers = 10;
	const float maxLayers = 15;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), v)));

	// height of each layer
	float layerHeight = 1.0 / numLayers;
	// depth of current layer
	float currentLayerHeight = 0;
	// shift of texture coordinates for each iteration
	vec2 dtex = gHeightScale * v.xy / v.z / numLayers;

 	// current texture coordinates
	vec2 currentTextureCoords = t;

	// depth from heightmap
	float heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;

	// while point is above surface
	while(heightFromTexture > currentLayerHeight) 
	{
		// go to the next layer
		currentLayerHeight += layerHeight; 
		// shift texture coordinates along V
		currentTextureCoords -= dtex;
		// new depth from heightmap
		heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;
   }
	// Start of Relief Parallax Mapping
	// decrease shift and height of layer by half
	vec2 deltaTexCoord = dtex / 2;
	float deltaHeight = layerHeight / 2;

	// return to the mid point of previous layer
	currentTextureCoords += deltaTexCoord;
	currentLayerHeight -= deltaHeight;

	// binary search to increase precision of Steep Paralax Mapping
	const int numSearches = 5;
	for(int i = 0; i < numSearches; i++)
	{
		// decrease shift and height of layer by half
		deltaTexCoord /= 2;
		deltaHeight /= 2;

		// new depth from heightmap
		heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;

		// shift along or agains vector V
		if(heightFromTexture > currentLayerHeight) // below the surface
		{
			currentTextureCoords -= deltaTexCoord;
			currentLayerHeight += deltaHeight;
		}
		else // above the surface
		{
			currentTextureCoords += deltaTexCoord;
			currentLayerHeight -= deltaHeight;
 		}
	}
	return currentTextureCoords;
}

vec2 occlusionPallaxMapping1(in vec3 v, in vec2 t)
{
	// determine optimal number of layers
	const float minLayers = 10;
	const float maxLayers = 15;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), v)));

	// height of each layer
	float layerHeight = 1.0 / numLayers;
	// current depth of the layer
	float curLayerHeight = 0;
	// shift of texture coordinates for each layer
	vec2 dtex = gHeightScale * v.xy / v.z / numLayers;

	// current texture coordinates
	vec2 currentTextureCoords = t;

	// depth from heightmap
	float heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;

	// while point is above the surface
	while(heightFromTexture > curLayerHeight) 
	{
		// to the next layer
		curLayerHeight += layerHeight; 
		// shift of texture coordinates
		currentTextureCoords -= dtex;
		// new depth from heightmap
		heightFromTexture = texture(NormTexSampler, currentTextureCoords).a;
	}
	// previous texture coordinates
	vec2 prevTCoords = currentTextureCoords + dtex;

	// heights for linear interpolation
	float nextH	= heightFromTexture - curLayerHeight;
	float prevH	= texture(NormTexSampler, prevTCoords).a - curLayerHeight + layerHeight;

	// proportions for linear interpolation
	float weight = nextH / (nextH - prevH);

	// interpolation of texture coordinates
	vec2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0-weight);
	// return result
	return finalTexCoords;
}


vec2 occlusionPallaxMapping2(in vec3 v, in vec2 t)
{
	int		nMaxSamples			= 15;
	int		nMinSamples			= 10;
	float	fHeightMapScale		= 0.1;

	int nNumSamples = int(mix(nMaxSamples, nMinSamples, abs(dot(vec3(0, 0, 1), v))));

	// height of each layer
	float fStepSize = 1.0 / float(nNumSamples);

	// Calculate the parallax offset vector max length.
	// This is equivalent to the tangent of the angle between the
	// viewer position and the fragment location.
	float fParallaxLimit = length(v.xy ) / v.z;

	// Scale the parallax limit according to heightmap scale.
	fParallaxLimit *= fHeightMapScale;						

	// Calculate the parallax offset vector direction and maximum offset.
	vec2 vOffsetDir = normalize(v.xy);
	vec2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Initialize the starting view ray height and the texture offsets.
	float fCurrRayHeight = 1.0;	
	vec2 vCurrOffset = vec2(0, 0);
	vec2 vLastOffset = vec2(0, 0);

	vec2 dx = dFdx(t);
	vec2 dy = dFdy(t);

	float fLastSampledHeight = 1;
	float fCurrSampledHeight = 1;

	int nCurrSample = 0;

	while ( nCurrSample < nNumSamples )
	{
		// Sample the heightmap at the current texcoord offset.  The heightmap 
		// is stored in the alpha channel of the height/normal map.
		//fCurrSampledHeight = tex2Dgrad( NH_Sampler, IN.texcoord + vCurrOffset, dx, dy ).a;
		fCurrSampledHeight = textureGrad(NormTexSampler, TexVertUV + vCurrOffset, dx, dy).a;

		// Test if the view ray has intersected the surface.
		if (fCurrSampledHeight > fCurrRayHeight)
		{
			// Find the relative height delta before and after the intersection.
			// This provides a measure of how close the intersection is to 
			// the final sample location.
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + fStepSize) - fLastSampledHeight;
			float ratio = delta1 / (delta1 + delta2);

			// Interpolate between the final two segments to 
			// find the true intersection point offset.
			vCurrOffset = ratio * vLastOffset + (1.0 - ratio) * vCurrOffset;
			
			// Force the exit of the while loop
			nCurrSample = nNumSamples + 1;	
		}
		else
		{
			// The intersection was not found.  Now set up the loop for the next
			// iteration by incrementing the sample count,
			nCurrSample ++;

			// take the next view ray height step,
			fCurrRayHeight -= fStepSize;
			
			// save the current texture coordinate offset and increment
			// to the next sample location, 
			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			// and finally save the current heightmap height.
			fLastSampledHeight = fCurrSampledHeight;
		}
	}
	// Calculate the final texture coordinate at the intersection point.
	return TexVertUV + vCurrOffset;
}

// Calculates lighting by Blinn-Phong model and Normal Mapping
// Returns color of the fragment
vec3 normalMappingLighting(in vec2 t, in vec3 l, in vec3 v)
{
	// restore normal from normal map
	vec3 n = normalize(texture(NormTexSampler, t).rgb * 2.0 - 1.0);
	vec3 d = texture(DiffTexSampler, t).rgb;
	float power = 0.3;
	// ambient lighting
	float iamb = 0.1;

	// diffuse lighting
	float idiff = clamp(dot(n, l), 0, 1);

	float ispec = clamp(dot(v + l, n), 0, 1) * power;

	return d * (iamb + idiff + ispec);
}


void main()
{
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(LightTangSpace);

	// Direction of the eye (from the fragment to the eye)
	vec3 v = normalize(EyeTangSpace);

	vec2 t;
	if(gSwitch == 0)
		t = TexVertUV;
	else if(gSwitch == 1)
		t = pallaxWithOffsetLimit(v, TexVertUV);
	else if(gSwitch == 2)
		t = steepPallaxMapping(v, TexVertUV);
	else if(gSwitch == 3)
		t = reliefPallaxMapping(v, TexVertUV);
	else if(gSwitch == 4)
		t = occlusionPallaxMapping1(v, TexVertUV);
	else
		t = occlusionPallaxMapping2(v, TexVertUV);

	color = normalMappingLighting(t, l, v);
}