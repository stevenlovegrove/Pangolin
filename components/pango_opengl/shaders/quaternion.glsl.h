
vec3 rotatePointByQuaternion( vec4 q, vec3 v ){
	return v + 2.0*cross(cross(v, q.xyz ) + q.w*v, q.xyz);
}

mat3 mat3FromQuaternion(vec4 q) {
    float xx = 2.0 * q.x * q.x;
    float xy = 2.0 * q.x * q.y;
    float xz = 2.0 * q.x * q.z;
    float xw = 2.0 * q.x * q.w;
    float yy = 2.0 * q.y * q.y;
    float yz = 2.0 * q.y * q.z;
    float yw = 2.0 * q.y * q.w;
    float zz = 2.0 * q.z * q.z;
    float zw = 2.0 * q.z * q.w;

    mat3 R = mat3(
        vec3(1.0-yy-zz, xy-zw, xz+yw), // column 1
        vec3(xy+zw, 1.0-xx-zz, yz-xw), // column 2
        vec3(xz-yw, yz+xw, 1.0-xx-yy)  // column 3
    );

    return R;
}
