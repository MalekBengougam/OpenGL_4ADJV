
ObjViewer_00
------------

Charge un OBJ dans un VBO
Les matrices monde et vue sont respectivement une rotation et une translation inverse (rappel, T(x)^-1 = T(-x))
Le fragment shader implémente une simple illumination diffuse lambertienne avec correction gamma explicite en lecture et ecriture.

ObjViewer_01
------------

Charge un OBJ en utilisant les VAO

ObjViewer_02
------------

Le fragment shader implémente l'equation d'illumination de Phong avec un Phong speculaire.

ObjViewer_03
------------

La matrice de vue est calculée a l'aide de la fonction LookAt inverse

ObjViewer_04
------------

La composante ambiante, précédemment une seule couleur unie, est remplacée par un calcul d'ambiance hémisphérique sur deux couleurs (ciel et sol).

ObjViewer_05
------------

Le fragment shader tient compte de deux lumières directionnelles de couleurs différentes

ObjViewer_06
------------

Charge un OBJ contenant plusieurs shapes (objects/geometry OBJ) et matériaux différents
Le code OpenGL active la conversion automatique de la sortie d'un fragment (gl_FragColor) en sRGB via glEnable(GL_FRAMEBUFFER_SRGB)
Le fragment shader est adapté en conséquence 

ObjViewer_07
------------

Le rendu 3D est effectué hors-écran à l'aide d'un Framebuffer Object (FBO)
La texture utilisée en tant que color-attachment du FBO est copiée dans le backbuffer via un shader simple appliqué à un quadrilatère plein écran (en NDC)

ObjViewer_08
------------

Le fragment shader de la seconde passe applique un simple filtre de luminance




