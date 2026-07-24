vec3 Linear2SRGB(vec3 linearRGB)
{
    vec3 sRGB = pow(linearRGB, vec3(1.0 / 2.2));
    
    return sRGB;
}


vec3 ComputeColorFromColorGradient(ColorGradientInfo info , float lerpFactor)
{
    int lastCursorIndex = info.currentCursorCount - 1;
    ColorGradientCursor firstCursor = info.colorGradientCursor[0];
    ColorGradientCursor lastCursor = info.colorGradientCursor[lastCursorIndex];
    
    if (lerpFactor <= firstCursor.lerpFactor)
    {
        return Linear2SRGB(vec3(firstCursor.R , firstCursor.G ,  firstCursor.B));
    } 
    if (lerpFactor >= lastCursor.lerpFactor)
    {
        return Linear2SRGB(vec3(lastCursor.R ,  lastCursor.G , lastCursor.B));
    }
    
    int i = 0;
    for (; i < lastCursorIndex ; i++)
    {
        if (lerpFactor >= info.colorGradientCursor[i].lerpFactor &&
            lerpFactor <= info.colorGradientCursor[i + 1].lerpFactor)
        {
            break;
        }
    }
    
    ColorGradientCursor frontCursor = info.colorGradientCursor[i];
    ColorGradientCursor backCursor = info.colorGradientCursor[i + 1];
    
    float ratio = (lerpFactor - frontCursor.lerpFactor) / (backCursor.lerpFactor - frontCursor.lerpFactor);
    ratio = clamp(ratio , 0.0 , 1.0);
    vec3 result ;
    result.x = mix(frontCursor.R ,backCursor.R , ratio); 
    result.y = mix(frontCursor.G ,backCursor.G , ratio); 
    result.z = mix(frontCursor.B ,backCursor.B , ratio); 
    return Linear2SRGB(result)  ;
}