#version 330 core
out vec4 FragColor;  
flat in vec4 ourColor;
flat in vec3 ourNormal;
in vec3 FragPos;
//in vec2 UV;
  
//uniform sampler2D ourTexture;
uniform vec3 lightColor;
uniform vec3 lightPos;  
uniform vec3 viewPos;

void main()
{
    //FragColor = ourColor;
    //FragColor = texture(ourTexture, UV) * ourColor;
    // FragColor = vec4(lightColor, 1.0) * ourColor;

    float ambientStrength = 0.1;
    float specularStrength = 0.7;

    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(ourNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0),32);
    vec3 specular = specularStrength * spec * lightColor;  
         
    vec4 result = vec4(ambient + diffuse + specular, 1.0) * ourColor;
    FragColor = result;
}
