float3 radians(float3 degrees)
{
    return degrees * 0.017453292519943295f;
}

float4x4 translateMatrix(float3 t)
{
    return float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(t.x, t.y, t.z, 1.0)
    );
}

float4x4 scaleMatrix(float3 s)
{
    return float4x4(
        float4(s.x, 0.0, 0.0, 0.0),
        float4(0.0, s.y, 0.0, 0.0),
        float4(0.0, 0.0, s.z, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );
}

float4x4 rotX(float a)
{
    float c = cos(a);
    float s = sin(a);
    return float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, c, s, 0.0),
        float4(0.0, -s, c, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );
}

float4x4 rotY(float a)
{
    float c = cos(a);
    float s = sin(a);
    return float4x4(
        float4(c, 0.0, -s, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(s, 0.0, c, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );
}

float4x4 rotZ(float a)
{
    float c = cos(a);
    float s = sin(a);
    return float4x4(
        float4(c, s, 0.0, 0.0),
        float4(-s, c, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );
}

float4x4 eulerRotationXYZ(float3 euler)
{
    return rotX(euler.x) * rotY(euler.y) * rotZ(euler.z);
}
