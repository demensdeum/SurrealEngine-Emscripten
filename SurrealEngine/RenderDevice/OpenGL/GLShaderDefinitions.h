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
    vec4 color;
    color.r = 1.0;
    color.g = 0.0;
    color.b = 0.0;
    gl_FragColor = color;
    })";
