// fragment
#version 330 core
out vec4 FragColor;

uniform vec3 wire_color;

void main()
{
	FragColor = vec4(wire_color, 1.0);
}