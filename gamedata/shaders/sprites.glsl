
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

// passed from vertex shader
in vec2 Texcoord;
in vec3 Position;
flat in uint PaletteIndex;
in vec2 SpriteTextureSize;
in vec2 SpriteTexelSize;

// result
out vec4 FinalColor;

vec4 fetchSpriteTexel(vec2 tc)
{
    // get color index in palette
    float pal_color = float(texture(tex_0, tc).r);

    if (pal_color < 0.5) // transparent
		discard;

    // fetch pixel color
    vec4 texelColor = texelFetch(tex_3, ivec2(int(pal_color), int(PaletteIndex)), 0);
    texelColor.a = 1.0;
	return texelColor;
}

// entry point
void main()
{
	vec4 texelColor = fetchSpriteTexel(Texcoord);
    FinalColor = clamp(texelColor, 0.0, 1.0);
}

#endif