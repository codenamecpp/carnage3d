
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;
uniform vec3 camera_position;

// attributes
in vec4 in_pos0; // position + size
in vec4 in_color0;

// pass to fragment shader
out vec4 FragColor;

// local constants
const float minPointScale = 0.1;
const float maxPointScale = 0.7;
const float maxDistance   = 50.0;

// entry point
void main() 
{
    FragColor = in_color0;

    float distanceFromCamera = distance(in_pos0.xyz, camera_position);
    float pointScale = 1.0 - (distanceFromCamera / maxDistance);
    pointScale = clamp(pointScale, minPointScale, maxPointScale);

	gl_PointSize = in_pos0.w * pointScale;
    gl_Position = view_projection_matrix * vec4(in_pos0.xyz, 1.0);
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform sampler2D tex_0;

// passed from vertex shader
in vec4 FragColor;

// result
out vec4 FinalColor;

// entry point
void main()
{
    FinalColor = vec4(1.0,1.0,1.0,1.0);
}

#endif