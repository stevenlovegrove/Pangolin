
vec3 rotate( vec4 q, vec3 v ) {
	return v + 2.0*cross(cross(v, q.xyz ) + q.w*v, q.xyz);
}

// Returns the 3x3 rotation matrix equivelent
// to the quaternion q
mat3 makeSo3(vec4 q) {
    float xx = 2.0 * q.x * q.x;
    float xy = 2.0 * q.x * q.y;
    float xz = 2.0 * q.x * q.z;
    float xw = 2.0 * q.x * q.w;
    float yy = 2.0 * q.y * q.y;
    float yz = 2.0 * q.y * q.z;
    float yw = 2.0 * q.y * q.w;
    float zz = 2.0 * q.z * q.z;
    float zw = 2.0 * q.z * q.w;
    return transpose(mat3(
        vec3(1.0-yy-zz, xy-zw, xz+yw), // column 1
        vec3(xy+zw, 1.0-xx-zz, yz-xw), // column 2
        vec3(xz-yw, yz+xw, 1.0-xx-yy)  // column 3
    ));
}

// Assembles 4x4 Se3 from rotation and translation
mat4 makeSe3(mat3 b_R_a, vec3 a_in_b) {
    return mat4(
        vec4(b_R_a[0], 0.0), // column 0
        vec4(b_R_a[1], 0.0), // column 1
        vec4(b_R_a[2], 0.0), // column 2
        vec4(a_in_b, 1.0)    // column 3
    );
}

// Returns the 4x4 transformation matrix that
// takes points from frame a into b.
mat4 makeSe3(vec4 b_quat_a, vec3 a_in_b) {
    return makeSe3(makeSo3(b_quat_a), a_in_b);
}

// Returns the inverse transformation of R_ba
mat3 invSo3(mat3 R_ba) {
    return transpose(R_ba);
}

// Returns the inverse transformation of T_ba
mat4 invSe3(mat4 T_ba) {
    mat3 R_ab = invSo3(mat3(T_ba));
    vec3 a_b = T_ba[3].xyz;
    vec3 b_a =  -R_ab * a_b;
    return makeSe3(R_ab, b_a);
}
