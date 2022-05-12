#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;

void main() {
	vec2 coordinate = vec2(TexCoord.x, 1.0 - TexCoord.y);
	vec3 col = texture(screenTexture, coordinate).rgb;
	FragColor = vec4(col, 1.0);
}
