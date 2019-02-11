#version 330 core

in  vec3 vPosition;
in  vec3 vColor;
in  vec3 vNormal;
in  vec2 vTexCoord;
in  vec3 vFaceIndecies;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 ViewMatrix;
uniform vec3 lightPos;

out vec4 color;
out vec2 texCoord;
out vec4 normal;

void main()
{
	
    texCoord = vTexCoord;
    normal = vec4(vNormal, 0);

 	vec4 v1 = ModelView*vec4(vPosition, 1.0);
	vec4 v2 = vec4(v1.xyz/v1.w , 1.0);
	vec4 v3 = Projection*ViewMatrix *v2;

	gl_Position = v3;

	vec4 vertPos_cameraspace = ViewMatrix * ModelView * vec4(vPosition, 1.0);

	vec3 V = vertPos_cameraspace.xyz / vertPos_cameraspace.w;
	
	vec3 N = (ViewMatrix * ModelView * vec4(vNormal, 0.0)).xyz;

	vec3 ambiColor = vColor;
	vec3 diffColor = vec3(0.8, 0.8, 0.8);
	vec3 specColor = vec3(0.3, 0.3, 0.3);

	vec3 N_norm = normalize(N);
	vec3 L_norm = normalize(( ViewMatrix  * vec4(lightPos, 1)).xyz - V);
	vec3 V_norm = normalize(-V);
	vec3 R_norm = reflect(-L_norm,N_norm);

	float lambertian = clamp(dot(L_norm, N_norm), 0.0, 1.0);
	float specular = clamp(dot(R_norm, V_norm), 0.0, 1.0);
	
	float shininess = 200;
	color = vec4((ambiColor + diffColor * lambertian + specColor * pow(specular, shininess)), 1.0);
}
