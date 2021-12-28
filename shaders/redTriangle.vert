#version 450

vec3 positions[3] = vec3[3](
  vec3(1.0f, 1.0f, 0.0f), 
  vec3(0.0f, -1.0f, 0.0f),
  vec3(-1.0f, 1.0f, 0.0f)
);

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 1.0);
}
