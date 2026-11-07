// Grass, dry patches, worn dirt and snow, painted from world position so it
// never repeats visibly. Colours come from the season (set by world.gd).
shader_type spatial;

uniform vec3 grass_a : source_color = vec3(0.30, 0.38, 0.18);
uniform vec3 grass_b : source_color = vec3(0.45, 0.42, 0.22);
uniform vec3 dirt : source_color = vec3(0.30, 0.23, 0.16);
uniform float dirt_amount = 0.2;
uniform float snow = 0.0;
uniform float wet = 0.0;
uniform sampler2D noise_big : repeat_enable, filter_linear_mipmap;
uniform sampler2D noise_small : repeat_enable, filter_linear_mipmap;

varying vec3 world_pos;

void vertex() {
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec2 p = world_pos.xz;
	float big = texture(noise_big, p * 0.011).r;
	float mid = texture(noise_small, p * 0.07).r;
	float fine = texture(noise_small, p * 0.45).r;

	vec3 grass = mix(grass_a, grass_b, smoothstep(0.38, 0.72, big));
	grass *= 0.78 + 0.4 * fine;
	float d = smoothstep(1.0 - dirt_amount - 0.14, 1.0 - dirt_amount + 0.14, mid * 0.5 + big * 0.5);
	vec3 colour = mix(grass, dirt * (0.8 + 0.35 * fine), d);

	// Snow settles unevenly: patchy at first, then everywhere.
	float s = clamp(snow * 1.35 - (1.0 - mid) * 0.35, 0.0, 1.0);
	colour = mix(colour, vec3(0.9, 0.92, 0.95) * (0.9 + 0.1 * fine), s);

	ALBEDO = colour;
	ROUGHNESS = mix(0.95, 0.4, wet * d * (1.0 - s));
	SPECULAR = 0.3;
}
