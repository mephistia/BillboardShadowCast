#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 viewProj;
uniform vec2 size;

out vec2 TexCoords;

void buildTree(vec4 position) {
	// posição na camera
	vec4 spawn = viewProj * position;
	// direção para somar ou subtrair ao criar vértices
	vec2 dir = size * 0.5f;

	// criar vértices
	// direita cima
	gl_Position = vec4(spawn.x + dir.x, spawn.y + 3*dir.y, spawn.z, spawn.w); // árvore é retangular, 3x a altura
	TexCoords = vec2(1, 0);
	EmitVertex();

	// esquerda cima
	gl_Position = vec4(spawn.x - dir.x, spawn.y + 3*dir.y, spawn.z, spawn.w);
	TexCoords = vec2(0, 0);
	EmitVertex();

	// direita baixo
	gl_Position = vec4(spawn.x + dir.x, spawn.y - dir.y, spawn.z, spawn.w);
	TexCoords = vec2(1, 1);
	EmitVertex();

	// esquerda baixo
	gl_Position = vec4(spawn.x - dir.x, spawn.y - dir.y, spawn.z, spawn.w);
	TexCoords = vec2(0, 1);
	EmitVertex();




	EndPrimitive();

}

void main() {

	buildTree(gl_in[0].gl_Position);

}