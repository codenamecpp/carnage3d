
//////////////////////////////////////////////////////////////////////////
#ifdef VERTEX_SHADER

// constants
uniform mat4 view_projection_matrix;

// attributes
in vec3 in_pos0;
in vec2 in_texcoord0;
in uint in_color0; // palette index
in uvec2 in_textureSize; // sprite texture size in pixels

// pass to fragment shader
out vec2 Texcoord;
out vec3 Position;
flat out uint PaletteIndex;
out vec2 SpriteTextureSize;
out vec2 SpriteTexelSize;

// entry point
void main() 
{
	Texcoord = in_texcoord0;
    Position = in_pos0;
    PaletteIndex = in_color0;
	SpriteTextureSize = vec2(in_textureSize);
	SpriteTexelSize = vec2(1.0 / SpriteTextureSize.x, 1.0 / SpriteTextureSize.y);

    vec4 vertexPosition = view_projection_matrix * vec4(in_pos0, 1.0);
    gl_Position = vertexPosition;
}

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef FRAGMENT_SHADER

uniform usampler2D tex_0;
uniform sampler2D tex_3; // palettes table
uniform bool enable_bilinear_filtering;

// passed from vertex shader
in vec2 Texcoord;
in vec3 Position;
flat in uint PaletteIndex;
in vec2 SpriteTextureSize;
in vec2 SpriteTexelSize;

// result
out vec4 FinalColor;

bool fetchSpriteTexel(vec2 tc, inout vec4 texelColor)
{
    // get color index in palette
    float pal_color = float(texture(tex_0, tc).r);

    if (pal_color < 0.5) // transparent
	{
		texelColor.a = 0.0;
		return false;
	}

    // fetch pixel color
    texelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    texelColor.a = 1.0;
	return true;
}

vec4 fetchSpriteTexelBiLinear()
{
    vec4 p0q0;
	if (!fetchSpriteTexel(Texcoord, p0q0))
		discard;

	// bilinear filtering
	// https://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL

    vec4 p1q0 = p0q0;
	fetchSpriteTexel(Texcoord + vec2(SpriteTexelSize.x, 0.0), p1q0);

    vec4 p0q1 = p0q0;
	fetchSpriteTexel(Texcoord + vec2(0.0, SpriteTexelSize.y), p0q1);

    vec4 p1q1 = p0q0;
	fetchSpriteTexel(Texcoord + SpriteTexelSize, p1q1);

    float a = fract(Texcoord.x * SpriteTextureSize.x); // Get Interpolation factor for X direction.
					// Fraction near to valid data.

    vec4 pInterp_q0 = mix(p0q0, p1q0, a); // Interpolates top row in X direction.
    vec4 pInterp_q1 = mix(p0q1, p1q1, a); // Interpolates bottom row in X direction.

    float b = fract(Texcoord.y * SpriteTextureSize.y);// Get Interpolation factor for Y direction.
    return mix(pInterp_q0, pInterp_q1, b); // Interpolate in Y direction.
}

// entry point
void main()
{
	vec4 texelColor;
	// no filtering
	if (!enable_bilinear_filtering)
	{
		if (!fetchSpriteTexel(Texcoord, texelColor))
			discard;
	}
	else
	{
		texelColor = fetchSpriteTexelBiLinear();
	}

    FinalColor = clamp(texelColor, 0.0, 1.0);
}

#endif