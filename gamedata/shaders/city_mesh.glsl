
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;
uniform usampler2D tex_1; // block frames table
uniform usampler2D tex_2; // palette indices table

// attributes
in vec3 in_pos0;
in vec3 in_texcoord0;
in uint in_color0; // remap index
in uint in_color1; // transparency flag

// pass to fragment shader
out vec2 Texcoord;
flat out float Transparency;
flat out float BlockTextureIndex;
flat out float PaletteIndex;

const float MeshHeightModifier = -0.15; // shift the geometry level slightly below the sprites to remove the zfighting

// entry point
void main() 
{
	Texcoord = in_texcoord0.xy;
    Transparency = float(in_color1);

    // get real block tile index
    BlockTextureIndex = float(texelFetch(tex_1, ivec2(int(in_texcoord0.z + 0.5), 0), 0).r);

    // get palette index for block tile
    PaletteIndex = float(texelFetch(tex_2, ivec2(int(4.0 * BlockTextureIndex + float(in_color0)), 0), 0).r);

    vec4 vertexPosition = view_projection_matrix * vec4(
		in_pos0.x, 
		in_pos0.y + MeshHeightModifier, 
		in_pos0.z, 1.0);

    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform usampler2DArray tex_0; // block texture
uniform sampler2D tex_3; // palettes table

// passed from vertex shader
in vec2 Texcoord;
flat in float Transparency;
flat in float BlockTextureIndex;
flat in float PaletteIndex;

// result
out vec4 FinalColor;

vec4 fetchBlockTexel(vec2 tc)
{
    // get color index in palette
    float pal_color = float(texture(tex_0, vec3(tc.x, tc.y, BlockTextureIndex)).r);

    if (Transparency > 0.5 && pal_color < 0.5) // transparent
		discard;

    vec4 texelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    texelColor.a = 1.0;
	return texelColor;
}

// entry point
void main()
{
	vec4 texelColor = fetchBlockTexel(Texcoord);
    FinalColor = clamp(texelColor, 0.0, 1.0);
}

#endif
