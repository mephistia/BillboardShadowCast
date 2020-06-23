#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse; // textura chão
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position; // pos da luz

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// info arvore
struct Tree {
    vec3 position; // ponto inicial da árvore
    sampler2D tex; // textura árvore
    vec2 size;
    vec2 texCoord; // mapeamento da textura
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos; // posição da view/câmera
uniform Material material;
uniform Light light;
uniform Tree tree;


// ponto onde o raio bate (algoritmo de Möller-Trumbore)
vec2 UV;
bool RayTriangleMT(
        // Origem e direção do raio
        vec3 lightOPos, vec3 lightDir, 
        // os vértices  do triângulo
        vec3 v0, vec3 v1, vec3 v2
    ) {
    vec3 v0v1 = v1 - v0; // edge 1
    vec3 v0v2 = v2 - v0; // edge 2

    vec3 p_ = cross(lightDir, v0v2); 
    float det = dot(v0v1, p_); // determinante

    // raio paralelo
    if (abs(det) < 0.00000001) {
        return false;
    }

    float f = 1 / det;
    vec3 t_ = lightOPos - v0;
    float u = f * dot(t_, p_);

    if (u < 0.00000001 || u > 1.0) {
        return false;
    }

    vec3 q_ = cross(t_, v0v1);
    float v = f * dot(lightDir, q_);

    if (v < 0.00000001 || u + v > 1) {
        return false;
    }

    float t = f * dot(v0v2, q_);

    // retorna baricentro
    UV = vec2(u,v);
    return true;

}

void main()

{
    // vértices da árvore
    vec3 treeTopRight;
    vec3 treeTopLeft;
    vec3 treeBottomRight;
    vec3 treeBottomLeft;
    vec2 treeDir = tree.size * 0.5f;

    vec4 groundTexColor = texture(material.diffuse, TexCoords);

    // ambient
    vec3 ambient = light.ambient * groundTexColor.rgb;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * groundTexColor.rgb; // (porcentagem de rgb da textura q vai ser mostrada)

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;

    // cor da sombra depende da arvore
    float shadow = 0.8 * diff;

    // definir os vértices do billboard                                                                 // texCoords:
    treeTopRight = vec3(tree.position.x + treeDir.x, tree.position.y + 3 * treeDir.y, tree.position.z); vec2 TR = vec2(1, 0); 
    treeTopLeft = vec3(tree.position.x - treeDir.x, tree.position.y + 3 * treeDir.y, tree.position.z);  vec2 TL = vec2(0, 0);  
    treeBottomRight = vec3(tree.position.x + treeDir.x, tree.position.y - treeDir.y, tree.position.z);  vec2 BR = vec2(1, 1);  
    treeBottomLeft = vec3(tree.position.x - treeDir.x, tree.position.y - treeDir.y, tree.position.z);   vec2 BL = vec2(0, 1); 


    if (RayTriangleMT(light.position, lightDir, treeTopRight, treeTopLeft, treeBottomRight)) {
            
        // usa o baricentro nas coordenadas de textura p/ definir coordenada do fragmento
        vec2 P = (1 - UV.x - UV.y) * TR + UV.x * TL + UV.y * BR;

        FragColor = vec4(result, 1.0) * (1 - texture(tree.tex, P).a * shadow);
    }

    else if (RayTriangleMT(light.position, lightDir, treeTopLeft, treeBottomRight, treeBottomLeft)) {

        vec2 P = (1 - UV.x - UV.y) * TL + UV.x * BR + UV.y * BL;

        FragColor = vec4(result, 1.0) * (1 - texture(tree.tex, P).a * shadow);
    }
    else
        FragColor = vec4(result,1.0);
}