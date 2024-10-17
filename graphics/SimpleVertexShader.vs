#version 330 core
in vec3 aPos;
in vec4 aColor;
in vec3 aNormal;
in vec2 aUV;

// Output data ; will be interpolated for each fragment.
flat out vec4 ourColor;
flat out vec3 ourNormal;
out vec3 FragPos;

//out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);

	ourColor = aColor;
	// Ineffecient for the shader to compute inverse for every vertex.
	// Move to CPU
    ourNormal = mat3(transpose(inverse(model))) * aNormal;
	// ourNormal = aNormal;  
	//UV = aUV;
}
