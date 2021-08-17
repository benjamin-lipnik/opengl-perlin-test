#version 330 core

out vec4 result;
in vec3 vertex_pos;
in vec3 mvp_vertex_pos;

uniform vec3 camera_pos;



void main() {
	float height = vertex_pos.z / 15;
	vec3 color = vec3(height,0,1-height); 

	result = vec4(color,1.0f);
}
