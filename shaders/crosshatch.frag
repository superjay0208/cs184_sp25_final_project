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
		// --- Ambient term (flat) ---
		vec3 ambient = ambientStrength * lightColor;

		// --- Diffuse (dot‐product) ---
		float diff = max(dot(Normal, normalize(-lightDir)), 0.0);

		diff = pow(diff, 0.5);

		// Quantize into 4 levels: 0,1,2,3
		int tone = int(floor(diff * 14.0));
		tone = clamp(tone, 0, 3);

		// Sample the cross‐hatch atlas:
		//   U = your mesh’s TexCoord.x
		//   V = (tone + 0.5) / 4.0  → pick middle of each horizontal band
		float v = (float(tone) + 0.5) / 4.0;
		float hatch = texture(crossHatchMap, vec2(TexCoord.x, v)).r;

		// Combine
		vec3 color = ambient + lightColor * hatch;

		FragColor = vec4(color, 1.0);
}
