// Flat Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

uniform sampler2D colorMap;

in float lightIntensity;
in vec4 vVaryingTexCoords;

// Output fragment color
out vec4 vFragColor;

void main(void)
{ 
	vec4 vTmpColor = texture(colorMap, vVaryingTexCoords.st);
	vFragColor.rgb = vTmpColor.rgb * lightIntensity;
	vFragColor.a = lightIntensity;
}