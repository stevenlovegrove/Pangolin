// Inigo Quilez, MIT licenced
// https://iquilezles.org/articles/boxocclusion/

#include "occlusion.glsl.h"

float boxOcclusion( in vec3 pos, in vec3 nor, in vec3 box[8] )
{
    // 8 points
    vec3 v[0] = normalize( box[0] );
    vec3 v[1] = normalize( box[1] );
    vec3 v[2] = normalize( box[2] );
    vec3 v[3] = normalize( box[3] );
    vec3 v[4] = normalize( box[4] );
    vec3 v[5] = normalize( box[5] );
    vec3 v[6] = normalize( box[6] );
    vec3 v[7] = normalize( box[7] );

    // 12 edges
    float k02 = dot( n, normalize( cross(v[2],v[0])) ) * acos( dot(v[0],v[2]) );
    float k23 = dot( n, normalize( cross(v[3],v[2])) ) * acos( dot(v[2],v[3]) );
    float k31 = dot( n, normalize( cross(v[1],v[3])) ) * acos( dot(v[3],v[1]) );
    float k10 = dot( n, normalize( cross(v[0],v[1])) ) * acos( dot(v[1],v[0]) );
    float k45 = dot( n, normalize( cross(v[5],v[4])) ) * acos( dot(v[4],v[5]) );
    float k57 = dot( n, normalize( cross(v[7],v[5])) ) * acos( dot(v[5],v[7]) );
    float k76 = dot( n, normalize( cross(v[6],v[7])) ) * acos( dot(v[7],v[6]) );
    float k37 = dot( n, normalize( cross(v[7],v[3])) ) * acos( dot(v[3],v[7]) );
    float k64 = dot( n, normalize( cross(v[4],v[6])) ) * acos( dot(v[6],v[4]) );
    float k51 = dot( n, normalize( cross(v[1],v[5])) ) * acos( dot(v[5],v[1]) );
    float k04 = dot( n, normalize( cross(v[4],v[0])) ) * acos( dot(v[0],v[4]) );
    float k62 = dot( n, normalize( cross(v[2],v[6])) ) * acos( dot(v[6],v[2]) );

    // 6 faces
    float occ = 0.0;
    occ += ( k02 + k23 + k31 + k10) * step( 0.0,  v0.z );
    occ += ( k45 + k57 + k76 + k64) * step( 0.0, -v4.z );
    occ += ( k51 - k31 + k37 - k57) * step( 0.0, -v5.x );
    occ += ( k04 - k64 + k62 - k02) * step( 0.0,  v0.x );
    occ += (-k76 - k37 - k23 - k62) * step( 0.0, -v6.y );
    occ += (-k10 - k51 - k45 - k04) * step( 0.0,  v0.y );

    return occ / 6.283185;
}
