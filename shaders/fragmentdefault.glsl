#version 150

// Données en entrée (par fragments et non par sommets)
// Les valeurs "in" sont interpolés linéairement entre chaque sommet du triangle
in vec3 varColor;
in vec3 fragPos;
in vec3 varNormal;
in vec4 varTexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

// Couleur de sortie du fragment
out vec4 outColor;

void main(void) {
    //outColor = vec4( normalize(varNormal), 1.0);

    // Différentes lignes à tester et comprendre:
    
    //ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(varNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    vec3 specular = specularStrength * spec * lightColor; 

    vec3 result = (ambient + diffuse + specular) * objectColor;

    outColor = vec4(result,1.0);
        
    //outColor = varTexCoord;
    //outColor = vec4 (0.0, 1.0, 1.0, 1.0);
    
    
}

