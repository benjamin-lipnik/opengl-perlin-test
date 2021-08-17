#include "bMatrix.h"

void bm_print_4x4_matrix(bm_Mat4x4 mat) {
    for(unsigned char r = 0; r < 4; r++) { //for each row in resulting matrix
        for(unsigned char c = 0; c < 4; c++) { //for each column in resulting matrix
            printf("%.2f, ", mat.values[c+(r<<2)]);
        }
        printf("\n");
    }
    printf("\n");
}

bm_Mat4x4 bm_product_4x4_4x4(bm_Mat4x4 left, bm_Mat4x4 right) {
    #define MAT_I(mat,r,c) (mat.values[c+(r<<2)])

    bm_Mat4x4 result = {0};

    for(unsigned char r = 0; r < 4; r++) { //for each row in resulting matrix
        for(unsigned char c = 0; c < 4; c++) { //for each column in resulting matrix
            for(unsigned char i = 0; i < 4; i++) { //for each component of value in resulting matrix
                MAT_I(result, r, c) += MAT_I(left, r, i) * MAT_I(right, i, c);
			}
		}
    }

    #undef MAT_I
    return result;
}

bm_Mat4x4 bm_scaling_matrix_4x4 (bm_Vec3f scale_vector) {
    return (bm_Mat4x4){
    	scale_vector.x, 0,              0,              0,
    	0,              scale_vector.y, 0,              0,
	    0,              0,              scale_vector.z, 0,
	    0,              0,              0,              1
    };
}

bm_Mat4x4 bm_translation_matrix(bm_Vec3f translation_vector) {
    return (bm_Mat4x4){
    	1, 0, 0, translation_vector.x,
	    0, 1, 0, translation_vector.y,
	    0, 0, 1, translation_vector.z,
	    0, 0, 0, 1
    };
}

bm_Mat4x4 bm_rotation_matrix(float angle_in_rad, bm_Vec3f rot_axis) {
	float cos_fi = cos(angle_in_rad);
	float sin_fi = sin(angle_in_rad);
	float not_cos_fi = 1.0f-cos_fi;

	return (bm_Mat4x4){
		cos_fi + pow(rot_axis.x, 2) * not_cos_fi, 
		rot_axis.x * rot_axis.y * not_cos_fi - rot_axis.z * sin_fi,
		rot_axis.x * rot_axis.z * not_cos_fi + rot_axis.y*sin_fi,
		0,
		
		rot_axis.y * rot_axis.x * not_cos_fi + rot_axis.z * sin_fi,
		cos_fi + pow(rot_axis.y, 2) * not_cos_fi,
		rot_axis.y * rot_axis.z * not_cos_fi - rot_axis.x * sin_fi,
		0,
		
		rot_axis.x * rot_axis.z * not_cos_fi - rot_axis.y * sin_fi,
		rot_axis.y * rot_axis.z * not_cos_fi + rot_axis.x * sin_fi,
		cos_fi + pow(rot_axis.z,2) * not_cos_fi,
		0,

		0,
		0,
		0,
		1
	};
}

bm_Mat4x4 bm_translate(bm_Mat4x4 input_matrix, bm_Vec3f translation_vector) {
  return bm_product_4x4_4x4(input_matrix, bm_translation_matrix(translation_vector));
}
bm_Mat4x4 bm_rotate(bm_Mat4x4 input_matrix, float angle, bm_Vec3f rotation_axes) {
  return bm_product_4x4_4x4(input_matrix, bm_rotation_matrix(angle, rotation_axes));
}
bm_Mat4x4 bm_scale(bm_Mat4x4 input_matrix, bm_Vec3f scaling_vector) {
  return bm_product_4x4_4x4(input_matrix, bm_scaling_matrix_4x4(scaling_vector));
}
bm_Mat4x4 bm_ortho(float left, float right, float bottom, float top, float d_near, float d_far) {
	return (bm_Mat4x4){
		2.0f / (right-left), 0,                     0,                       (right+left)/(left-right),
		0,                   2.0f / (top - bottom), 0,                       (top+bottom) / (bottom - top),
		0,                   0,                     2.0f / (d_near - d_far), (d_far + d_near) / (d_near - d_far),
		0,                   0,                     0,                       1
	};
}

bm_Mat4x4 bm_perspective(float fov, float aspect_ratio, float near, float far) {
	float f = 1.0f / tan(fov * 0.5f);
	return (bm_Mat4x4){
		f / aspect_ratio,     0,     0,                         0,
		0,                    f,     0,                         0,
		0,                    0,     (far+near)/(near-far),     (2*far*near)/(near-far),
		0,                    0,     -1,                        0
	};
}

bm_Vec3f bm_normalize(bm_Vec3f vec) {
	//vsako komponento je treba delit z dolzino vektorja
	float magnitude = sqrt(pow(vec.x,2)+pow(vec.y,2)+pow(vec.z,2));
	return (bm_Vec3f) {vec.x/magnitude, vec.y/magnitude, vec.z/magnitude};
}

bm_Vec3f bm_cross(bm_Vec3f a, bm_Vec3f b) {
	return (bm_Vec3f) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
bm_Mat4x4 bm_look_at(bm_Vec3f pos, bm_Vec3f target, bm_Vec3f up) {
	bm_Vec3f m_forward = bm_normalize((bm_Vec3f){pos.x - target.x, pos.y - target.y, pos.z - target.z});
	bm_Vec3f m_right = bm_normalize(bm_cross(up, m_forward));
	bm_Vec3f m_up = bm_cross(m_forward, m_right);	

	bm_Mat4x4 direction_mat = {
		m_right.x,   m_right.y,   m_right.z,   0,
		m_up.x,	     m_up.y,      m_up.z,      0,
		m_forward.x, m_forward.y, m_forward.z, 0,
	        0,	     0,           0,           1
	};
	bm_Mat4x4 position_mat = {
		1, 0, 0, -pos.x,
		0, 1, 0, -pos.y,
		0, 0, 1, -pos.z,
		0, 0, 0, 1
	};
	return bm_product_4x4_4x4(direction_mat, position_mat);
}

bm_Vec3f bm_mat_vec_product(bm_Mat4x4 mat, bm_Vec3f vec) {
	//|a b c d|   |x|   |ax + by + cz + d|
	//|e f g h| * |y| = |ex + fy + gz + h|
	//|i j k l|   |z|   |ix + jy + kz + l|
	//|m n o p|   |1|   |mx + ny + oz + p|
	float* mv = mat.values;
	bm_Vec4f result = {
		mv[0]*vec.x  + mv[1]*vec.y  + mv[2]*vec.z  + mv[3],
		mv[4]*vec.x  + mv[5]*vec.y  + mv[6]*vec.z  + mv[7],
		mv[8]*vec.x  + mv[9]*vec.y  + mv[10]*vec.z + mv[11],
		mv[12]*vec.x + mv[13]*vec.y + mv[14]*vec.z + mv[15]
	};
	return (bm_Vec3f){
		result.x / result.w,  
		result.y / result.w,
		result.z / result.w
	};
}
