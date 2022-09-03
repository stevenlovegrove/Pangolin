uniform sampler2D u_font_atlas;
uniform sampler2D u_font_offsets;

const float font_height = 32.0;

float screenPxRange(vec2 tex_coord) {
    const float pxRange = 2.0;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_font_atlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(tex_coord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float sdf_font(vec4 atlas_offset, vec2 pos)
{
    float atlas_dim = textureSize(u_font_atlas, 0).x;

    vec2 p = vec2(1,-1) * clamp2(
        pos/atlas_dim, vec2(0.0, 0.0),
        vec2(atlas_offset.z, -atlas_offset.w)
    );

    vec2 uv = atlas_offset.xy + p;
    vec2 tex = vec2(uv.x, uv.y);
    vec3 msd = texture(u_font_atlas, tex).xyz;
    float sd = median(msd.r, msd.g, msd.b);
    return screenPxRange(tex)*(sd - 0.5);
}

float sdf_font(int char_id, vec2 pos )
{
    vec4 font_offset = texelFetch(u_font_offsets, ivec2(char_id, 0), 0);
    vec2 screen_offset = texelFetch(u_font_offsets, ivec2(char_id, 1), 0).xy;
    return sdf_font(font_offset, pos - vec2(font_height/2.0) - screen_offset*font_height);
}

float font_color(vec4 atlas_offset, vec2 pos )
{
    return clamp(sdf_font(atlas_offset, pos) + 0.5, 0.0, 1.0);
}

float font_color(int char_id, vec2 pos )
{
    return clamp(sdf_font(char_id, pos) + 0.5, 0.0, 1.0);
}
