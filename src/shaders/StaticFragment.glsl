#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 FragColor;

void main() {
    vec2 testUvs = uv;
    testUvs.x *= 10.0f;
    testUvs.y *= 10.0f;
    vec4 diffuseColor = texture(tex0, testUvs);

	vec3 n = normalize(norm);
	vec3 l = normalize(light);
	float diffuseIntensity = clamp(dot(n, l) + 0.1, 0, 1);

	FragColor = diffuseColor * diffuseIntensity;
}
