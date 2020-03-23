#version 120

attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoords;
attribute vec3 a_Color;		// vertex color, suppose une valeur par defaut de (1, 1, 1) sans alpha

uniform mat4 u_WorldMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_TexCoords;
varying vec3 v_Color; 		// vertex color, suppose une valeur par defaut de (1, 1, 1) sans alpha

void main(void)
{
	v_TexCoords = a_TexCoords;

	// approx. decompression gamma, les couleurs des vertices ont ete saisies dans l'espace colorimetrique
	// du moniteur (en sRGB) il faut donc convertir en RGB lineaire pour que les maths soient corrects
	v_Color = pow(a_Color, vec3(2.2));

	v_Position = vec3(u_WorldMatrix * vec4(a_Position, 1.0));
	// note: techniquement il faudrait passer une normal matrix du C++ vers le GLSL
	// pour les raisons que l'on a vu en cours. A defaut on pourrait la calculer ici
	// mais les fonctions inverse() et transpose() n'existe pas dans toutes les versions d'OpenGL 2.0
	// on suppose ici que la matrice monde -celle appliquee a v_Position- est orthogonale (sans deformation des axes)
	v_Normal = mat3(u_WorldMatrix) * a_Normal;

	gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_WorldMatrix * vec4(a_Position, 1.0);
}