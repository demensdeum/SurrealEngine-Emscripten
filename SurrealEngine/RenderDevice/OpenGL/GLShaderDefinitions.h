#pragma once

// Shaders taken from VulkanRenderDevice, and modified a bit to fit OpenGL 3.3

const std::string vertexShaderCode =
    R"(#version 100
attribute vec4 vertex;
attribute vec2 uvIn;
varying vec2 uvOut;
void main() {
   gl_Position = vertex;
   uvOut = uvIn;
})";

const std::string fragmentShaderCode = 
    R"(#version 100
    precision mediump float;
    varying vec2 uvOut;
    uniform sampler2D texture;
    void main() {
        gl_FragColor = texture2D(texture, uvOut);
    })";
