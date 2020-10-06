
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos0;
in vec4 in_color0;

// pass to fragment shader
out vec4 FragColor;

// entry point
void main() 
{
    FragColor = in_color0;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

// passed from vertex shader
in vec4 FragColor;

// result
out vec4 FinalColor;

// entry point
void main()
{
    FinalColor = FragColor;
}

#endif