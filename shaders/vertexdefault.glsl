#version 150
/*
 * Paramètres généraux
 */
// Matrices
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 MVP;
uniform mat4 normalMatrix;

// Position des sources
uniform vec3 keyLightPosition;
uniform vec3 fillLightPosition;
uniform vec3 backLightPosition;

/*
 * Données en entrée
 */
in vec3 inPosition;
in vec3 inNormal;
in vec4 inTexCoord;

/*
 * Données en sortie
 */
// Sommet
out vec3 varNormal;
out vec4 varTexCoord;

// Eclairage
out vec3 lightDirInView[3];
out vec3 halfVecInView[3];


void computeLightingVectorsInView(in vec3 posInView, in vec3 lightPosition, out vec3 lightDir, out vec3 halfVec)
{
    // Fonction à écrire par les étudiants

    lightDir = normalize(lightPosition - posInView);
    vec3 viewDir  = normalize(vec3(0.0f, 0.0f, 0.0f) - posInView);
    halfVec = normalize(viewDir + lightDir);

}

void main(void) {

// Calculez ici les informations géométriques (lightDirInView et halfVecInView) pour les sources de lumière

vec3 posInView = vec3( modelViewMatrix * vec4(inPosition.xyz, 1.0));

computeLightingVectorsInView(posInView, keyLightPosition , lightDirInView[0], halfVecInView[0]);
computeLightingVectorsInView(posInView, fillLightPosition, lightDirInView[1], halfVecInView[1]);
computeLightingVectorsInView(posInView, backLightPosition, lightDirInView[2], halfVecInView[2]);

// Calcul du sommet
    varNormal = normalize((normalMatrix * vec4(inNormal,0.0)).xyz);
    varTexCoord = inTexCoord;
    gl_Position = MVP*vec4(inPosition.xyz, 1.0);

}
