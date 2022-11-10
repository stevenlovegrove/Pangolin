// for a ray, x = (0,0,0)^T + lambda * ray
// and the parameters (n,d)^T = (nx,ny,nz,d)^T of the
// plane equation such that (n,d).(x,1)=0,
// returns the parameter lambda
float intersectRayPlane(vec3 ray, vec4 plane_eqn)
{
    return -plane_eqn.w / dot(plane_eqn.xyz, ray);
}

// simplification of above for z0 plane
float intersectRayPlaneZ0(vec3 c_w, vec3 dir_w)
{
    // c_w + depth * dir_w == (_,_,0.0), therefore...
    return -c_w.z / dir_w.z;
}
