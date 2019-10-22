
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos0;
in vec3 in_texcoord0;
in float in_color0;
in float in_color1;

// pass to fragment shader
out vec3 Texcoord;
out vec3 Position;
flat out float RemapIndex;
flat out float Transparency;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Position = in_pos0;
    RemapIndex = in_color0;
    Transparency = in_color1;

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0f);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform usampler2DArray tex_0; // block texture
uniform isamplerBuffer tex_1; // block frames table
uniform isamplerBuffer tex_2; // palette indices table
uniform isamplerBuffer tex_3; // palettes table

// passed from vertex shader
in vec3 Texcoord;
in vec3 Position;
flat in float RemapIndex;
flat in float Transparency;

// result
out vec4 FinalColor;

// entry point
void main()
{
    // get real block tile index
    float block_texture_index = texelFetch(tex_1, int(Texcoord.z + 0.5)).r;

    // get color index in palette
    float pal_color = texture(tex_0, vec3(Texcoord.x, Texcoord.y, block_texture_index)).r;

    if (Transparency > 0.5 && pal_color < 0.5) // old school alpha test
        discard;

    // get palette index for block tile
    float palette_index = texelFetch(tex_2, int(4.0 * block_texture_index + RemapIndex)).r;

    // fetch pixel color at least
    vec4 pixelColor = texelFetch(tex_3, int(palette_index * 256 + pal_color)) / 255.0;
    pixelColor.a = 1.0f;
    FinalColor = clamp(pixelColor, 0.0f, 1.0f);
}

#endif
