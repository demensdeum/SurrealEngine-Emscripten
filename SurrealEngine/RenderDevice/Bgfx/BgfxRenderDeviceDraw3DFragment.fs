$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texture0,  0);

void main() {
    gl_FragColor = texture2D(s_texture0, v_texcoord0);
    //gl_FragColor = vec4(1, 0, 0, 0);
}