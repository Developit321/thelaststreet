// Photo-textured wood, stone and shingles, wrapped by world position (no UV
// maps needed), with damp grime near the ground and snow that settles only
// on surfaces facing up: roofs, sills, the tops of fences and sheds.
shader_type spatial;

uniform sampler2D albedo_tex : source_color, filter_linear_mipmap_anisotropic, repeat_enable;
uniform sampler2D normal_tex : hint_normal, filter_linear_mipmap_anisotropic, repeat_enable;
uniform sampler2D rough_tex : hint_default_white, filter_linear_mipmap_anisotropic, repeat_enable;
uniform vec3 tint : source_color = vec3(1.0);
uniform float scale = 0.5;       // 1 / the texture's real size in metres
uniform float grime = 0.6;       // damp and dirt creeping up from the ground
uniform float snow = 0.0;

varying vec3 world_pos;
varying vec3 world_normal;

void vertex() {
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	world_normal = normalize((MODEL_MATRIX * vec4(NORMAL, 0.0)).xyz);
}

vec4 triplanar(sampler2D tex, vec3 p, vec3 w) {
	return texture(tex, p.zy) * w.x + texture(tex, p.xz) * w.y + texture(tex, p.xy) * w.z;
}

void fragment() {
	vec3 w = pow(abs(world_normal), vec3(4.0));
	w /= (w.x + w.y + w.z);
	vec3 p = world_pos * scale;

	vec3 colour = triplanar(albedo_tex, p, w).rgb * tint;
	float rough = triplanar(rough_tex, p, w).r;
	vec3 nmap = triplanar(normal_tex, p, w).rgb;

	float low = 1.0 - smoothstep(0.2, 1.4, world_pos.y);
	colour *= mix(vec3(1.0), vec3(0.6, 0.64, 0.52), low * grime);

	float up = smoothstep(0.3, 0.7, world_normal.y);
	float s = clamp(snow * 1.4, 0.0, 1.0) * up;
	colour = mix(colour, vec3(0.9, 0.92, 0.96), s);

	ALBEDO = colour;
	ROUGHNESS = mix(rough, 0.85, s);
	NORMAL_MAP = mix(nmap, vec3(0.5, 0.5, 1.0), s * 0.8);
}
