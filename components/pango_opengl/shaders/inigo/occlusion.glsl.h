// fully visible front facing Triangle occlusion
float ftriOcclusion( in vec3 pos, in vec3 nor, in vec3 v0, in vec3 v1, in vec3 v2 )
{
    vec3 a = normalize( v0 - pos );
    vec3 b = normalize( v1 - pos );
    vec3 c = normalize( v2 - pos );

    return (dot( nor, normalize( cross(a,b)) ) * acos( dot(a,b) ) +
            dot( nor, normalize( cross(b,c)) ) * acos( dot(b,c) ) +
            dot( nor, normalize( cross(c,a)) ) * acos( dot(c,a) ) ) / 6.283185;
}

float triOcclusionWithClipping( in vec3 pos, in vec3 nor, in vec3 v0, in vec3 v1, in vec3 v2, in vec4 plane )
{
    if( dot( v0-pos, cross(v1-v0,v2-v0) ) < 0.0 ) return 0.0;  // back facing

    float s0 = dot( vec4(v0,1.0), plane );
    float s1 = dot( vec4(v1,1.0), plane );
    float s2 = dot( vec4(v2,1.0), plane );

    float sn = sign(s0) + sign(s1) + sign(s2);

    vec3 c0 = clip( v0, v1, plane );
    vec3 c1 = clip( v1, v2, plane );
    vec3 c2 = clip( v2, v0, plane );

    // 3 (all) vertices above horizon
    if( sn>2.0 )
    {
        return ftriOcclusion(  pos, nor, v0, v1, v2 );
    }
    // 2 vertices above horizon
    else if( sn>0.0 )
    {
        vec3 pa, pb, pc, pd;
              if( s0<0.0 )  { pa = c0; pb = v1; pc = v2; pd = c2; }
        else  if( s1<0.0 )  { pa = c1; pb = v2; pc = v0; pd = c0; }
        else/*if( s2<0.0 )*/{ pa = c2; pb = v0; pc = v1; pd = c1; }
        return fquadOcclusion( pos, nor, pa, pb, pc, pd );
    }
    // 1 vertex above horizon
    else if( sn>-2.0 )
    {
        vec3 pa, pb, pc;
              if( s0>0.0 )   { pa = c2; pb = v0; pc = c0; }
        else  if( s1>0.0 )   { pa = c0; pb = v1; pc = c1; }
        else/*if( s2>0.0 )*/ { pa = c1; pb = v2; pc = c2; }
        return ftriOcclusion(  pos, nor, pa, pb, pc );
    }
    // zero (no) vertices above horizon

    return 0.0;
}

// fully visible front acing Quad occlusion
float fquadOcclusion( in vec3 pos, in vec3 nor, in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3 )
{
    vec3 a = normalize( v0 - pos );
    vec3 b = normalize( v1 - pos );
    vec3 c = normalize( v2 - pos );
    vec3 d = normalize( v3 - pos );

    return (dot( nor, normalize( cross(a,b)) ) * acos( dot(a,b) ) +
            dot( nor, normalize( cross(b,c)) ) * acos( dot(b,c) ) +
            dot( nor, normalize( cross(c,d)) ) * acos( dot(c,d) ) +
            dot( nor, normalize( cross(d,a)) ) * acos( dot(d,a) ) ) / 6.283185;
}
