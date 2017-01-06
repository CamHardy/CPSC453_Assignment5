#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"

using namespace glm;
/*
mat4 rotateAbout(vec3 axis, float radians)
{
	mat4 matrix;

	matrix[0][0] = cos(radians) + axis.x*axis.x*(1-cos(radians));
	matrix[1][0] = axis.x*axis.y*(1-cos(radians))-axis.z*sin(radians);
	matrix[2][0] = axis.x*axis.z*(1-cos(radians)) + axis.y*sin(radians);

	matrix[0][1] = axis.y*axis.x*(1-cos(radians)) + axis.z*sin(radians);
	matrix[1][1] = cos(radians) + axis.y*axis.y*(1-cos(radians));
	matrix[2][1] = axis.y*axis.z*(1-cos(radians)) - axis.x*sin(radians);

	matrix[0][2] = axis.z*axis.x*(1-cos(radians)) - axis.y*sin(radians);
	matrix[1][2] = axis.z*axis.y*(1-cos(radians)) + axis.x*sin(radians);
	matrix[2][2] = cos(radians) + axis.z*axis.z*(1-cos(radians));

	return matrix;
}
*/

class Camera{
public:
	vec3 dir;
	vec3 up;
	vec3 right;
	vec3 pos;
	vec3 polarPos;
	vec3 lookingAt;
	float zoomLimit;

	Camera();
	Camera(vec3 _polarPos, vec3 _lookingAt, float _zoomLimit);

	void pol2cart();
	mat4 getMatrix();

	void translateCamera(float up, float around, float theOtherOne);

};










#endif
