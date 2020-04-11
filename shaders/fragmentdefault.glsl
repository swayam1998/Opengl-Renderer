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
    
    float cosTheta = abs(dot(normal, lightVector));
    float cosAlpha = abs(dot(normal, halfVector));
    vec3 colorFactor = (kd + ks*pow(cosAlpha, ns) ) * cosTheta;
    
	return colorFactor;
}

void main(void) {

    vec3 fragColor = vec3(0.0, 0.0, 0.0);
	// Calculer fragColor;

    fragColor = keyLightColor  * blinnPhongLighting(materialKd, materialKs, materialNs, varNormal, lightDirInView[0], halfVecInView[0]) + 
                fillLightColor * blinnPhongLighting(materialKd, materialKs, materialNs, varNormal, lightDirInView[1], halfVecInView[1]) +
                backLightColor * blinnPhongLighting(materialKd, materialKs, materialNs, varNormal, lightDirInView[2], halfVecInView[2]); 

    outColor = vec4( fragColor*globalColorFactor, 1.);

}

