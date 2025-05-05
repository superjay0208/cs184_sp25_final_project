#version 330 core

in  vec3 FragPos;
in  vec3 Normal;
in  vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D crossHatchMap;   // bound to GL_TEXTURE0

uniform vec3 lightDir;            // e.g. normalize(vec3(0.0, 1.0, 0.3))
uniform vec3 lightColor;          // e.g. vec3(1.2)
uniform float ambientStrength;     // e.g. 0.2

void main() {

		vec3 ambient = ambientStrength * lightColor;


		float diff = max(dot(Normal, normalize(-lightDir)), 0.0);

		diff = pow(diff, 0.5);


		int tone = int(floor(diff * 64.0));
		tone = clamp(tone, 1, 3);


		float v = (float(tone) + 0.5) / 4.0;
		float hatch = texture(crossHatchMap, vec2(TexCoord.x, v)).r;

		vec3 color = ambient + lightColor * hatch;

		FragColor = vec4(color, 1.0);
}
