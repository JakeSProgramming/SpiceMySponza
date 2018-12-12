#version 330

uniform mat4 combinedMatrix;
uniform mat4 modelMatrix;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 texCoord;

out vec3 P;
out vec3 N;

void main(void)
{	
	P = mat4x3(modelMatrix) * vec4(vertexPosition, 1.0);
	N = normalize(mat3(modelMatrix) * vertexNormal);

	gl_Position = combinedMatrix * vec4(vertexPosition, 1.0);
}
