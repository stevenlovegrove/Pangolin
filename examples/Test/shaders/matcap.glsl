uniform sampler2D u_matcap;

vec3 matcap(vec3 normal)
{
    vec2 t = (normal.xy + vec2(1.0,1.0)) / 2.0;
    return texture(u_matcap, t).xyz;
}
