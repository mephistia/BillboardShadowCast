#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 viewProj;
uniform vec2 size;

out vec2 TexCoords;

void buildTree(vec4 position) {
	// posi��o na camera
	vec4 spawn = viewProj * position;
	// dire��o para somar ou subtrair ao criar v�rtices
	vec2 dir = size * 0.5f;

	// criar v�rtices
	// direita cima
	gl_Position = vec4(spawn.x + dir.x, spawn.y + 3*dir.y, spawn.z, spawn.w); // �rvore � retangular, 3x a altura
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