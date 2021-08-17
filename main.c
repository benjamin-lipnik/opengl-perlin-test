#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "bMatrix.h"
#include "shader_loader.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


//PERLIN
static int SEED = 0;
static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y)
{
    int tmp = hash[(y + SEED) % 256];
    return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y)
{
	if(x<0)x=-x;
	if(y<0)y=-y;

    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth)
{
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

//GAME

const char* const shader_paths[] = {
	"shaders/a.vs",
	"shaders/b.fs",
	"shaders/b_nf.fs"
};

char* vertex_shader_path;
char* fragment_shader_path;

uint32_t shader;
uint8_t wireframe = 0;
uint8_t face_culling = 1;
uint8_t fog = 1;
uint8_t unlock_frame_cap = 0;
uint8_t debug_print = 0;
float off_x = 0, off_y = 0;

void genMap(int width, int length, float offset_x, float offset_y, uint32_t vbo) {
	uint32_t map_size = width*length*3;
	float* map = malloc(sizeof(float) * map_size);
	for(int x = 0; x < width; ++x) {
		for(int z = 0; z < length; z++) {
			int pos = x * length + z;

			map[pos * 3 + 0] = (x / (float)width - 0.5f)  * 200;
			map[pos * 3 + 1] = (z / (float)length - 0.5f) * 200;
			map[pos * 3 + 2] = 15.0f * perlin2d(x+offset_x,z+offset_y,0.05f,1);

			//printf("%.2f,%.2f,%.2f\n", map[pos * 3 + 0], map[pos * 3 + 1], map[pos * 3 + 2]);
		}
		//printf("\n");
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * map_size, map,  GL_DYNAMIC_DRAW);
	free(map);
}

float game_time = 0;
float delta_time = 0;

void update_mvp() {
	//mvp
	bm_Mat4x4 model = bm_identity_4x4;
	//model = bm_translate(model, (bm_Vec3f){0,0,-1});
	//model = bm_rotate(model, DEG_TO_RAD(40.0f), (bm_Vec3f){-1,0,0});
	//model = bm_rotate(model, game_time/3, (bm_Vec3f){0,0,1});

	bm_Mat4x4 view = bm_identity_4x4;
	bm_Vec3f camera_pos = {0,-15,-35};
	view = bm_translate(view, camera_pos);
	view = bm_rotate(view, DEG_TO_RAD(80.0f), (bm_Vec3f){-1,0,0});


	bm_Mat4x4 projection = bm_identity_4x4;
	projection = bm_perspective(DEG_TO_RAD(100), 1.0f, 0.1f, 1000.0f);

	bm_Mat4x4 mvp = bm_product_4x4_4x4(bm_product_4x4_4x4(projection, view), model);
	glUniformMatrix4fv(glGetUniformLocation(shader, "mvp"), 1, GL_TRUE,  mvp.values);
	glUniform3fv(glGetUniformLocation(shader, "camera_pos"), 1, &camera_pos);
}

void reload_shader(uint8_t force_reload) {
	static time_t last_change;	

	struct stat vertex_shader_stat;
	struct stat fragment_shader_stat;

	stat(vertex_shader_path, &vertex_shader_stat);
	stat(fragment_shader_path, &fragment_shader_stat);
	if(force_reload || (fragment_shader_stat.st_mtime > last_change || vertex_shader_stat.st_mtime > last_change)) {
		last_change = (fragment_shader_stat.st_mtime > vertex_shader_stat.st_mtime) ? fragment_shader_stat.st_mtime : vertex_shader_stat.st_mtime;
		printf("Shader changed, recompiling\n");			
		//recompile both shaders		
		uint32_t new_shader = sl_compile_and_link_shaders(vertex_shader_path, fragment_shader_path);
		glDeleteProgram(shader);
		shader = new_shader;
		glUseProgram(shader);
	} 
}

void handle_loop_input(GLFWwindow* window) {
	const float move_speed = 100.0f * delta_time; 
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		off_x -= move_speed;
	}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		off_x += move_speed;
	}
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		off_y += move_speed;
	}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		off_y -= move_speed;
	}
}
void handle_single_input(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;
	if(key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_W)
        wireframe = !wireframe;
	if(key == GLFW_KEY_C)
		face_culling = !face_culling;
	if(key == GLFW_KEY_F) {
		fog = !fog;
		if(fog) {
			fragment_shader_path = shader_paths[1];
			reload_shader(1);
		}
		else {
			fragment_shader_path = shader_paths[2];
			reload_shader(1);
		}
	}
	if(key == GLFW_KEY_D) {
		debug_print = !debug_print;
	}
	if(key == GLFW_KEY_U) {
		unlock_frame_cap = !unlock_frame_cap;
		if(unlock_frame_cap) {
			glfwSwapInterval(0);
		}
		else {
			glfwSwapInterval(1);	
		}
	}

}

int main() {
	srand(time(NULL));
	SEED = rand();

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(700, 700, "Haightmapping", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, handle_single_input);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));

	//map generator
	int width = 200;
	int length = 200;
	int map_size = width * length * 3;
	uint32_t indices_count = (width-1) * (length-1) * 6;
	uint32_t* indices = malloc(sizeof(uint32_t) * indices_count);

	for(int x = 0, c = 0; x < (width-1); ++x) {
		for(int z = 0; z < (length-1); ++z) {
			uint32_t pos = x * length + z;

			indices[c * 6 + 0] = pos;
			indices[c * 6 + 1] = pos + width;
			indices[c * 6 + 2] = pos + 1;

			indices[c * 6 + 3] = pos + 1;
			indices[c * 6 + 4] = pos + width;
			indices[c * 6 + 5] = pos + 1 + width;

			c++;
		}
	}

	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	uint32_t vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(float)*3,0);
	glEnableVertexAttribArray(0);

	uint32_t ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices_count, indices,  GL_STATIC_DRAW);

	free(indices);

	vertex_shader_path = shader_paths[0];
	fragment_shader_path = shader_paths[1];


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glUseProgram(shader);

	//glfwSwapInterval(0);
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		static float last_frame_time = 0;
		game_time = glfwGetTime();
		delta_time = game_time - last_frame_time;
		last_frame_time = game_time;

		static float fps;
		fps = 0.9f * fps + 0.1f * (1.0f / delta_time);

		static float dbg_print_timer = 0;
		if(game_time - dbg_print_timer > 0.1f && debug_print) {
			dbg_print_timer = game_time;
			printf("FPS: %.2f\n", fps);
		}

		reload_shader(0);

		static float last_off_x = 1;
		static float last_off_y = 1;

		if((off_x != last_off_x) || (off_y != last_off_y)) {
			last_off_x = off_x;
			last_off_y = off_y;
			genMap(width,length,off_x,off_y,vbo);
		}

		handle_loop_input(window); 
		update_mvp();
		
		if(wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if(face_culling) {
			glEnable(GL_CULL_FACE);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		/* Render here */
		glClearColor(0.1f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDrawArrays(GL_LINES, 0, map_size);
		glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
