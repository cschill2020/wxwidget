#version 330 core
in vec3 aPos;

uniform mat4 MVP;

void main()
{
	// Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * vec4(aPos, 1.0);
}
