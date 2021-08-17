#version 330 core

out vec4 result;
in vec3 vertex_pos;
in vec3 mvp_vertex_pos;

uniform vec3 camera_pos;

float dist_sqr(vec3 a, vec3 b) {
	return pow((a.x-b.x),2) + pow((a.y-b.y),2) + pow((a.z-b.z),2);
}
float dist(vec3 a, vec3 b) {
	return sqrt(dist_sqr(a,b));
}

vec3 apply_fog(vec3 rgb, float distance) {
	float fog_ammount = 1.0f - exp(-distance * 0.02f);
	vec3 fog_color = vec3(0, 0, 0);
	return mix(rgb, fog_color, fog_ammount);
}

void main() {
	float height = vertex_pos.z / 15;
	vec3 color = vec3(height,0,1-height); 

	color = apply_fog(color, dist(mvp_vertex_pos, vertex_pos));	

	result = vec4(color,1.0f);
}
