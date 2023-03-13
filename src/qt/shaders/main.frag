#version 410 core

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_color;

uniform sampler2D tex;

void main()
{
    vec4 _color = v_color;

    // Integrate texture color with the current vertex color
    if (v_texcoord.x > 0.0 || v_texcoord.y > 0.0)
    {
        vec4 _texColor = texture(tex, v_texcoord.xy);

        if (_texColor.a == 0.0) discard;

        _color *= _texColor;
    }

    o_color = _color;
}
