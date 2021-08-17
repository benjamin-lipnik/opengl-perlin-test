#ifndef BMATRIX
#define BMATRIX

#include <stdio.h>
#include <math.h>

#ifndef M_PI
	#define M_PI (3.14f)
#endif
#define RAD_TO_DEG(x) ((x)*180.0f / (M_PI))
#define DEG_TO_RAD(x) ((x)*M_PI / (180.0f))

typedef struct {
    float x,y,z;
} bm_Vec3f;

typedef struct {
    float x,y,z,w;
} bm_Vec4f;

typedef struct {
    float values[4*4];
}bm_Mat4x4;

typedef struct {
    float values[4*1];
}bm_Mat4x1;

static const bm_Mat4x4 bm_identity_4x4 = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
};

void bm_print_4x4_matrix(bm_Mat4x4 mat);
bm_Mat4x4 bm_product_4x4_4x4(bm_Mat4x4 left, bm_Mat4x4 right);
bm_Mat4x4 bm_scaling_matrix_4x4 (bm_Vec3f scale_vector);
bm_Mat4x4 bm_translation_matrix(bm_Vec3f translation_vector);
bm_Mat4x4 bm_rotation_matrix(float angle_in_rad, bm_Vec3f rotation_axes);
bm_Mat4x4 bm_look_at(bm_Vec3f pos, bm_Vec3f target, bm_Vec3f up);

bm_Mat4x4 bm_translate(bm_Mat4x4 input_matrix, bm_Vec3f translation_vector);
bm_Mat4x4 bm_rotate(bm_Mat4x4 input_matrix, float angle, bm_Vec3f rotation_axes);
bm_Mat4x4 bm_scale(bm_Mat4x4 input_matrix, bm_Vec3f scaling_vector);
bm_Mat4x4 bm_ortho(float left, float right, float bottom, float top, float d_near, float d_far);
bm_Mat4x4 bm_perspective(float fov, float aspect_ratio, float near, float far);

bm_Vec3f bm_normalize(bm_Vec3f vec);
bm_Vec3f bm_cross(bm_Vec3f a, bm_Vec3f b);
bm_Vec3f bm_mat_vec_product(bm_Mat4x4 mat, bm_Vec3f vec);

#endif
