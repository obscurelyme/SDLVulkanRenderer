#!/bin/sh

glslc ./shaders/shader.vert -o ./shaders/vert.spv
glslc ./shaders/redTriangle.vert -o ./shaders/redTriangleVert.spv
glslc ./shaders/redTriangle.frag -o ./shaders/redTriangleFrag.spv
glslc ./shaders/triangleMesh.vert -o ./shaders/triangleMesh.spv
glslc ./shaders/shader.frag -o ./shaders/frag.spv
