
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec2 in_pos0;
in vec2 in_texcoord0;
in vec4 in_color0;

// pass to fragment shader
out vec2 Texcoord;
out vec4 FragColor;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    FragColor = in_color0;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 0.0, 1.0);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;

// passed from vertex shader
in vec2 Texcoord;
in vec4 FragColor;

// result
out vec4 FinalColor;

// entry point
void main()
{
    vec4 pixelColor = texture(tex_0, Texcoord) * FragColor;
    FinalColor = pixelColor;
}

#endif