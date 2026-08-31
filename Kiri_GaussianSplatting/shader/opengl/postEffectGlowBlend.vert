#include "version.glsl"
#include "define.glsl"

out vec2 texCoord;

void main() {

    /*
    {
        [-1,-1],
        [3, -1],
        [-1, 3]
    }
    */
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    
    gl_Position = vec4(x, y, 0.0, 1.0);
    
    // NDC -> UV [0.0, 1.0] 
    texCoord = vec2(x * 0.5 + 0.5, y * 0.5 + 0.5);
}