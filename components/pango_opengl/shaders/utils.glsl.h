float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

vec2 clamp2(vec2 v, vec2 low, vec2 high)
{
    return vec2(
        clamp(v.x, low.x, high.x),
        clamp(v.y, low.y, high.y)
    );
}

vec4 composite(vec4 top_layer, vec4 bottom_layer)
{
    float alpha = top_layer.a;
//    return alpha * top_layer + (1.0 - alpha) * bottom_layer;
    return vec4(alpha * top_layer.xyz + (1.0 - alpha) * bottom_layer.xyz, max(alpha,bottom_layer.a));
}
