#version 330 core

layout(location = 0) in vec3 pos;
uniform mat4 mvp;

out vec3 vertex_pos;
out vec3 mvp_vertex_pos;

void main() {
	gl_Position = mvp * vec4(pos,1);
	vertex_pos = pos;
	mvp_vertex_pos = vec3(gl_Position);
}
