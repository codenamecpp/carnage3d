
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos0;
in vec2 in_texcoord0;
in uint in_color0; // palette index

// pass to fragment shader
out vec2 Texcoord;
out vec3 Position;
flat out uint PaletteIndex;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Position = in_pos0;
    PaletteIndex = in_color0;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform usampler2D tex_0;
uniform sampler2D tex_3; // palettes table

// passed from vertex shader
in vec2 Texcoord;
in vec3 Position;
flat in uint PaletteIndex;

// result
out vec4 FinalColor;

// entry point
void main()
{
    // get color index in palette

    float pal_color = float(texture(tex_0, Texcoord).r);

    if (pal_color < 0.5) // old school alpha test
        discard;

    // fetch pixel color
    vec4 pixelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    pixelColor.a = 1.0;
    FinalColor = clamp(pixelColor, 0.0, 1.0);
}

#endif