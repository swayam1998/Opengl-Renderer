#version 150
const float globalColorFactor = 0.75;

// material
uniform vec3 materialKd;
uniform vec3 materialKs;
uniform float materialNs;

// fragment
in vec3 varNormal;
in vec4 varTexCoord;

// lumieres
in vec3 lightDirInView[3];
in vec3 halfVecInView[3];

// material
uniform vec3 keyLightColor;
uniform vec3 fillLightColor;
uniform vec3 backLightColor;

// resultat
out vec4 outColor;

// --------------------

vec3 blinnPhongLighting (in vec3 kd, in vec3 ks, in float ns, in vec3 normal, in vec3 lightVector, in vec3 halfVector){
    // Fonction à écrire par les étudiants
	return vec3(0.0, 0.0, 0.0);
}

void main(void) {

    vec3 fragColor = vec3(0.0, 0.0, 0.0);
	// Calculer fragColor;

    outColor = vec4( fragColor*globalColorFactor, 1.);

}

