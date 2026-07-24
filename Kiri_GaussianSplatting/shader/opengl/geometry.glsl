
// return range 0~1
float GetShapeMaskLerpFactor(float shapeSize, vec4 shapeScaleXYZ , vec4 shapeCenter , vec4 inputPos){
    vec3 normalizedDist = (inputPos.xyz - shapeCenter.xyz) / shapeScaleXYZ.xyz;
    float dist = length(normalizedDist);
    return dist / shapeSize ;
}

float invert01(float x)
{
    return 1.0 - x;
}

mat4 translateMatrix(vec3 t)
{
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0
    );
}

mat4 scaleMatrix(vec3 s)
{
    return mat4(
        s.x, 0.0, 0.0, 0.0,
        0.0, s.y, 0.0, 0.0,
        0.0, 0.0, s.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotX(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, c, s, 0.0,
        0.0, -s, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotY(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat4(
         c, 0.0, -s, 0.0,
         0.0, 1.0, 0.0, 0.0,
         s, 0.0, c, 0.0,
         0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotZ(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat4(
        c, s, 0.0, 0.0,
       -s, c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 eulerRotationXYZ(vec3 euler)
{
    return rotX(euler.x) * rotY(euler.y) * rotZ(euler.z);
}