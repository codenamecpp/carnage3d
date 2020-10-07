
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
uniform bool enable_bilinear_filtering;

// constants

const float BlockTextureSize = 64.0; // width and height of block texture
const float BlockTexelSize = 1.0 / BlockTextureSize;

// passed from vertex shader
in vec2 Texcoord;
flat in float Transparency;
flat in float BlockTextureIndex;
flat in float PaletteIndex;

// result
out vec4 FinalColor;

bool fetchBlockTexel(vec2 tc, inout vec4 texelColor)
{
    // get color index in palette
    float pal_color = float(texture(tex_0, vec3(tc.x, tc.y, BlockTextureIndex)).r);

    if (Transparency > 0.5 && pal_color < 0.5) // transparent
	{
		texelColor.a = 0.0;
        return false;
	}

    texelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    texelColor.a = 1.0;
	return true;
}

vec4 fetchBlockTexelBiLinear()
{
    vec4 p0q0;
	if (!fetchBlockTexel(Texcoord, p0q0))
		discard;

	// bilinear filtering
	// https://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL

    vec4 p1q0 = p0q0;
	fetchBlockTexel(Texcoord + vec2(BlockTexelSize, 0.0), p1q0);

    vec4 p0q1 = p0q0;
	fetchBlockTexel(Texcoord + vec2(0.0, BlockTexelSize), p0q1);

    vec4 p1q1 = p0q0;
	fetchBlockTexel(Texcoord + vec2(BlockTexelSize, BlockTexelSize), p1q1);

    float a = fract(Texcoord.x * BlockTextureSize); // Get Interpolation factor for X direction.
					// Fraction near to valid data.

    vec4 pInterp_q0 = mix(p0q0, p1q0, a); // Interpolates top row in X direction.
    vec4 pInterp_q1 = mix(p0q1, p1q1, a); // Interpolates bottom row in X direction.

    float b = fract(Texcoord.y * BlockTextureSize);// Get Interpolation factor for Y direction.
    return mix(pInterp_q0, pInterp_q1, b); // Interpolate in Y direction.
}

// entry point
void main()
{
	vec4 texelColor;
	// no filtering
	if (!enable_bilinear_filtering)
	{
		if (!fetchBlockTexel(Texcoord, texelColor))
			discard;
	}
	else
	{
		texelColor = fetchBlockTexelBiLinear();
	}

    FinalColor = clamp(texelColor, 0.0, 1.0);
}

#endif
