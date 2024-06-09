#pragma once

// Shaders taken from VulkanRenderDevice, and modified a bit to fit OpenGL 3.3

const std::string vertexShaderCode =
    R"(#version 100
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

attribute vec4 vertex;
attribute vec2 uvIn;

varying vec2 uvOut;

void main() {
   // Correctly multiply the vertex position by the combined transformation matrices
   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;
   uvOut = uvIn;
}
)";

const std::string fragmentShaderCode = 
	R"(#version 100
    precision mediump int;
    precision mediump float;
    precision mediump sampler2D;
    precision mediump samplerCube;
    varying mediump vec2 uvOut;
    uniform sampler2D texture;
    uniform float brightness;
    void main() {
    vec4 color = texture2D(texture, uvOut);
    color.r = brightness * color.r;
    color.g = brightness * color.g;
    color.b = brightness * color.b;
    if (color.a < 0.5) {
    	discard;
    }
    gl_FragColor = color;
    })";
