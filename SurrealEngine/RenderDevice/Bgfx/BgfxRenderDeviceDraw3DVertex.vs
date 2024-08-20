$input a_position, a_texcoord0
$output v_texcoord0

uniform mat4 objectToProjectionMatrix;

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(objectToProjectionMatrix, vec4(a_position, 1.0));
    v_texcoord0 = a_texcoord0;
}