#version 450

layout(location = 0) out vec3 fragColor;

vec3 positions[3] = vec3[3](vec3(0.5f, 0.5f, 0.0f), vec3(0.0f, -0.5f, 0.0f), vec3(-0.5f, 0.5f, 0.0f));

layout(push_constant) uniform constants {
  vec4 data;
  mat4 renderMatrix;
}
PushConstants;

void main() {
  gl_Position = PushConstants.renderMatrix * vec4(positions[gl_VertexIndex], 1.0f);
  fragColor = vec3(1.0f, 1.0f, 1.0f);
}
