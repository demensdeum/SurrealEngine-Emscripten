#pragma once

// Shaders taken from VulkanRenderDevice, and modified a bit to fit OpenGL 3.3

const std::string vertexShaderCode =
    R"(#version 100
attribute vec4 vertex;

void main() {
   gl_Position = vertex;
}
)";

const std::string fragmentShaderCode = 
	R"(#version 100
    precision mediump int;
    precision mediump float;
    precision mediump sampler2D;
    precision mediump samplerCube;
    void main() {
    gl_FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    })";
