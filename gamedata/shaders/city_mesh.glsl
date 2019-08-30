
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos0;
in vec3 in_texcoord0;
in vec4 in_color0;

// pass to fragment shader
out vec3 Texcoord;
out vec4 FragColor;
out vec3 Position;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Position = in_pos0;
    FragColor = in_color0;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0f);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2DArray tex_0;
uniform bool enable_texture_mapping;

// passed from vertex shader
in vec3 Texcoord;
in vec4 FragColor;
in vec3 Position;

// result
out vec4 FinalColor;

// entry point
void main()
{
    vec4 pixelColor = vec4(1.0, 1.0, 1.0, 1.0);
    if (enable_texture_mapping)
    {
        pixelColor = texture(tex_0, Texcoord);

        if (FragColor.a < 1.0f && pixelColor.a < 1.0f) // old school alpha test
            discard;
    }
    else
    {
        pixelColor = FragColor;
    }

    FinalColor = clamp(pixelColor, 0.0f, 1.0f);
}

#endif