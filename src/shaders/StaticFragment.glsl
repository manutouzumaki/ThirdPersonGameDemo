#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 FragColor;

void main() {
    vec2 scaleUvs = uv;
    scaleUvs.x *= 100.0f;
    scaleUvs.y *= 100.0f;
    vec4 diffuseColor = texture(tex0, scaleUvs) - vec4(0.1, 0.2, 0.2, 0);

	vec3 n = normalize(norm);
	vec3 l = normalize(light);
	float diffuseIntensity = clamp(dot(n, l) + 0.1, 0, 1);

	FragColor = diffuseColor * diffuseIntensity;
}
