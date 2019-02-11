#version 330 core

in vec4 color;
in vec2 texCoord;
in vec4 normal;

out vec4 fColor;
out vec4 fNormal;


uniform sampler2D texture;
uniform int isShadow;

void main()
{
	if (isShadow == 1) {
		fColor = vec4(0, 0, 0, 1);
	} 
    else{
	     fColor = mix(texture2D( texture, texCoord ) , color, 0.3) ;
	}
    fNormal = normal;
}

