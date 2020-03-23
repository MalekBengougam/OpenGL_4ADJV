#version 120

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_TexCoords;
varying vec3 v_Color;		// vertex color, suppose une valeur par defaut de (1, 1, 1) sans alpha

struct Material {
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	float Shininess;
};
uniform Material u_Material;

uniform vec3 u_CameraPosition;

uniform sampler2D u_DiffuseTexture;

// calcul du facteur diffus, suivant la loi du cosinus de Lambert
float Lambert(vec3 N, vec3 L)
{
	return max(0.0, dot(N, L));
}

// calcul du facteur speculaire, methode de Phong
float Phong(vec3 N, vec3 L, vec3 V, float shininess)
{
	// reflexion du vecteur incident I (I = -L)
	// suivant la loi de ibn Sahl / Snell / Descartes
	vec3 R = reflect(-L, N);
	return pow(max(0.0, dot(R, V)), shininess);
}

// calcul du facteur speculaire, methode Blinn-Phong
float BlinnPhong(vec3 N, vec3 L, vec3 V, float shininess)
{
	// reflexion inspire du modele micro-facette (H approxime la normale de la micro-facette)
	vec3 H = normalize(L + V);
	return pow(max(0.0, dot(N, H)), shininess);
}

void main(void)
{
	// Dans ce premier exemple, les proprietes de la lumiere sont definies en dur
	// direction fixe de la lumiere (alignee sur l'axe forward du monde)
	const vec3 L = normalize(vec3(0.0, 0.0, 1.0));
	const vec3 lightColor = vec3(0.0, 1.0, 1.0);
	const float attenuation = 1.0; // on suppose une attenuation nulle ici
								   // theoriquement, l'attenuation naturelle est proche de 1 / distance²

	vec3 N = normalize(v_Normal);
	vec3 V = normalize(u_CameraPosition - v_Position);

	vec4 baseTexel = texture2D(u_DiffuseTexture, v_TexCoords);
	// decompression gamma, les couleurs des texels ont ete specifies dans l'espace colorimetrique
	// du moniteur (en sRGB) il faut donc convertir en RGB lineaire pour que les maths soient corrects
	baseTexel.rgb = pow(baseTexel.rgb, vec3(2.2));
	vec3 baseColor = baseTexel.rgb * v_Color;

	// les couleurs diffuse et speculaire traduisent l'illumination directe de l'objet
	vec3 diffuseColor = baseColor * u_Material.DiffuseColor * Lambert(N, L);
	vec3 specularColor = u_Material.SpecularColor * BlinnPhong(N, L, V, u_Material.Shininess);
	
	vec3 directColor = (diffuseColor + specularColor) * lightColor * attenuation;

	// la couleur ambiante traduit une approximation de l'illumination indirecte de l'objet
	vec3 ambientColor = baseColor * u_Material.AmbientColor;
	
	vec3 indirectColor = ambientColor;

	vec3 color = directColor + indirectColor;

	// correction gamma
	color = pow(color, vec3(1.0 / 2.2));

	gl_FragColor = vec4(color, 1.0);
}