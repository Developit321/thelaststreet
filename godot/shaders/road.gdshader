// The muddy street: churned earth, two dark cart ruts, puddles that shine
// when it's wet, and snow that stays out of the ruts.
shader_type spatial;

uniform float road_center = 54.0;   // street y of the middle of the road
uniform float road_half = 4.0;
uniform vec3 mud : source_color = vec3(0.32, 0.25, 0.18);
uniform vec3 mud_dark : source_color = vec3(0.18, 0.14, 0.10);
uniform float wet = 0.3;
uniform float snow = 0.0;
uniform sampler2D noise_big : repeat_enable, filter_linear_mipmap;
uniform sampler2D noise_small : repeat_enable, filter_linear_mipmap;

varying vec3 world_pos;

void vertex() {
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec2 p = world_pos.xz;
	float across = (world_pos.z - road_center) / road_half;  // -1..1 across the road
	float wobble = (texture(noise_big, vec2(p.x * 0.02, 0.5)).r - 0.5) * 0.25;
	float a = across + wobble;
	// Two ruts where the cart wheels run.
	float ruts = exp(-pow((a - 0.42) / 0.11, 2.0)) + exp(-pow((a + 0.42) / 0.11, 2.0));
	float fine = texture(noise_small, p * 0.5).r;
	float mid = texture(noise_small, p * 0.08).r;
	float big = texture(noise_big, p * 0.02).r;

	vec3 colour = mix(mud, mud_dark, clamp(ruts * 0.8 + (1.0 - mid) * 0.35, 0.0, 1.0));
	colour *= 0.8 + 0.35 * fine;
	// Grass creeping in at the edges.
	float edge = smoothstep(0.75, 1.0, abs(a) + (fine - 0.5) * 0.3);
	colour = mix(colour, vec3(0.30, 0.34, 0.18), edge * 0.7);

	// Puddles collect in the ruts and dips when it's wet.
	float puddle = smoothstep(0.62, 0.7, big * 0.55 + ruts * 0.4 + mid * 0.2) * wet;
	colour = mix(colour, vec3(0.08, 0.08, 0.07), puddle * 0.7);

	// Snow lies on the road but gets churned away along the ruts.
	float s = clamp(snow * 1.3 - ruts * 0.55 - (1.0 - mid) * 0.25, 0.0, 1.0);
	colour = mix(colour, vec3(0.86, 0.87, 0.88) * (0.9 + 0.1 * fine), s);

	ALBEDO = colour;
	ROUGHNESS = mix(mix(0.9, 0.55, wet), 0.05, puddle) * (1.0 - s * 0.2);
	METALLIC = 0.0;
	SPECULAR = mix(0.3, 0.9, puddle);
}
