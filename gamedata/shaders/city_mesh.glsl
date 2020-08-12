
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;
uniform isamplerBuffer tex_1; // block frames table
uniform isamplerBuffer tex_2; // palette indices table

// attributes
in vec3 in_pos0;
in vec3 in_texcoord0;
in int in_color0; // remap index
in int in_color1; // transparency flag

// pass to fragment shader
out vec3 Texcoord;
out vec3 Position;
flat out float Transparency;
flat out float BlockTextureIndex;
flat out float PaletteIndex;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Position = in_pos0;
    Transparency = in_color1;

    // get real block tile index
    BlockTextureIndex = texelFetch(tex_1, int(Texcoord.z + 0.5)).r;

    // get palette index for block tile
    PaletteIndex = texelFetch(tex_2, int(4.0 * BlockTextureIndex + in_color0)).r;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0f);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform usampler2DArray tex_0; // block texture
uniform sampler2D tex_3; // palettes table

// passed from vertex shader
in vec3 Texcoord;
in vec3 Position;
flat in float Transparency;
flat in float BlockTextureIndex;
flat in float PaletteIndex;

// result
out vec4 FinalColor;

// entry point
void main()
{
    // get color index in palette
    float pal_color = texture(tex_0, vec3(Texcoord.x, Texcoord.y, BlockTextureIndex)).r;

    if (Transparency > 0.5 && pal_color < 0.5) // old school alpha test
        discard;

    // fetch pixel color at least
    vec4 pixelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    pixelColor.a = 1.0f;
    FinalColor = clamp(pixelColor, 0.0f, 1.0f);
}

#endif
