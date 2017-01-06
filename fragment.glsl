// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

in vec3 FragNormal;
in vec2 FragUV;
in vec4 spacePos;

uniform sampler2D texSphere;
uniform bool diffuse;

void main(void) {

	//FragmentColour = vec4((vec3(FragUV, 0.0)), 1);
	if(diffuse) {
		vec4 sunColor = vec4(1.0);
		vec3 lightRay = normalize(vec3(0.0) - spacePos.xyz); // technically this should iterate and use the center of all light objects
		FragmentColour = texture(texSphere, FragUV) * sunColor * max(0.2, dot(FragNormal, lightRay));
	}
	else FragmentColour = texture(texSphere, FragUV);
}
