#include "struct.glsl"

float GetValueFromBezierCurve(BezierCurveInfo info, float lerpFactor)
{
    int lastPointIndex = info.currentPointCount - 1;
    BezierPoint firstPoint = info.bezierPointInfo[0];
    BezierPoint lastPoint  = info.bezierPointInfo[lastPointIndex];
    
    if (lerpFactor <= firstPoint.x)
    {
        return firstPoint.y;
    }
    if (lerpFactor >= lastPoint.x)
    {
        return lastPoint.y;
    }
    
    int i = 0;
    for (; i < lastPointIndex; i++)
    {
        if (lerpFactor >= info.bezierPointInfo[i].x &&
            lerpFactor <= info.bezierPointInfo[i + 1].x)
        {
            break;
        }
    }
    
    BezierPoint p0 = info.bezierPointInfo[i];
    BezierPoint p3 = info.bezierPointInfo[i + 1];
    
    float segmentWidth = p3.x - p0.x;
    float handleOffset = segmentWidth * 0.5f;
    
    vec2 cp0 = vec2(p0.x, p0.y);
    vec2 cp1 = vec2(handleOffset, p0.y);
    vec2 cp2 = vec2(handleOffset, p3.y);
    vec2 cp3 = vec2(p3.x, p3.y);
    
    
    //float cp0 = p0.y;
    //float cp1 = p0.y; 
    //float cp2 = p3.y; 
    //float cp3 = p3.y;
    
    float t = (lerpFactor - p0.x) / segmentWidth;
    
    //return t;
    
    float t2 = t * t;
    float t3 = t2 * t;
    
    float oneMinusT = 1.0f - t;
    float oneMinusT2 = oneMinusT * oneMinusT;
    float oneMinusT3 = oneMinusT2 * oneMinusT;
    
    vec2 value =
          oneMinusT3 * cp0
        + 3.0f * oneMinusT2 * t * cp1
        + 3.0f * oneMinusT * t2 * cp2
        + t3 * cp3;

    return clamp(value.y, 0.0, 1.0);
}