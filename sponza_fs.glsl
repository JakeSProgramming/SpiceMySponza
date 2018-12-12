#version 330

in vec3 P;
in vec3 N;

out vec4 fragment_colour;

void main(void)
{
	fragment_colour = vec4(N,1);
}
