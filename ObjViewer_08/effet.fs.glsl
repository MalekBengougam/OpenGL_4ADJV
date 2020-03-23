#version 120

uniform float u_Time;
uniform sampler2D u_Texture;

varying vec2 v_UV;

//#define LINEAR 1

void main(void)
{
	// ces poids sont a utiliser avec une couleur lineaire (RGB et pas sRGB)
	const vec3 luminanceLinearWeights = vec3(0.2126, 0.7152, 0.0722);

	// ces poids sont a utiliser avec une couleur sRGB aussi dite "perceptuelle"
	const vec3 luminancePerceptualWeights = vec3(0.299, 0.587, 0.114);

	float t = mod(u_Time / 4.0, 1.0);

	vec4 originalColor = texture2D(u_Texture, v_UV);

#ifdef LINEAR
	// l'image que l'on reçoit en entree est deja lineaire
	vec3 greyColor = vec3(dot(originalColor.rgb, luminanceLinearWeights));
#else
	// l'image que l'on reçoit en entree est en sRGB (ce qui est le cas ici)
	vec3 greyColor = vec3(dot(originalColor.rgb, luminancePerceptualWeights));
#endif

	vec3 color = mix(originalColor.rgb, greyColor, t);

	// encore une fois, comme GL_FRAMEBUFFER_SRGB est actif sur le back buffer
	// la conversion lineaire RGB vers sRGB gamma est faite automatiquement
	gl_FragColor = vec4(color, originalColor.a);
}