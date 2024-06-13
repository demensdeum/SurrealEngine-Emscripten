#pragma once

#include <string>

const std::string drawComplexSurfaceVertexShadersCode =
    R"(#version 100
attribute vec4 vertex;
attribute vec2 uvIn;
varying vec2 uvOut;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;
    uvOut = uvIn;
})";

const std::string drawComplexSurfaceFragmentShadersCode = 
    R"(#version 100
    precision mediump int;
    precision mediump float;
    precision mediump sampler2D;
    precision mediump samplerCube;
    varying mediump vec2 uvOut;
    uniform sampler2D texture;
    void main() {
        //gl_FragColor = vec4(uvOut[1], uvOut[0], 0.0, 1.0);
        gl_FragColor = texture2D(texture, uvOut);
    })";
    