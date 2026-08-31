#include "uniforms.glsl"

//vec3 ComputeColorFromSH(vec4 positionWorld)

vec3 ComputeColorFromSH(vec3 positionLocal)
{
	// The implementation is loosely based on code for 
	// "Differentiable Point-Based Radiance Fields for 
	// Efficient View Synthesis" by Zhang et al. (2022)
	vec3 result = SH_C0 * u_splatElement.sh[0];
   
	if (u_renderInfo.shDegree > 0)
	{
        mat4 invModelMatrix = inverse(u_renderInfo.modelMatrix);    
        
        vec3 splatPosition  = u_splatElement.position ;
        vec4 cameraPosition = vec4(u_renderInfo.cameraPos.xyz, 1.0);
        vec4 cameraPosLocal = invModelMatrix * cameraPosition;
        
        //vec3 dirWorld  = positionWorld.xyz - cameraPosition;
        //vec3 dirLocal  = normalize(invModelMatrix * dirWorld);
        vec3 dirLocal = normalize(positionLocal - cameraPosLocal.xyz);
        
		float x = dirLocal.x ;
		float y = dirLocal.y ;
		float z = dirLocal.z ;
        
		result += SH_C1* (-u_splatElement.sh[1] * y + u_splatElement.sh[2] * z - u_splatElement.sh[3] * x);
        if (u_renderInfo.shDegree > 1)
		{
			float xx = x * x, yy = y * y, zz = z * z;
			float xy = x * y, yz = y * z, xz = x * z;
			result = result +
				SH_C2[0] * xy * u_splatElement.sh[4] +
				SH_C2[1] * yz * u_splatElement.sh[5] +
				SH_C2[2] * (2.0f * zz - xx - yy) * u_splatElement.sh[6] +
				SH_C2[3] * xz * u_splatElement.sh[7] +
				SH_C2[4] * (xx - yy) * u_splatElement.sh[8];
        
			if (u_renderInfo.shDegree > 2)
			{
				result = result +
					SH_C3[0] * y * (3.0f * xx - yy) * u_splatElement.sh[9] +
					SH_C3[1] * xy * z * u_splatElement.sh[10] +
					SH_C3[2] * y * (4.0f * zz - xx - yy) * u_splatElement.sh[11] +
					SH_C3[3] * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * u_splatElement.sh[12] +
					SH_C3[4] * x * (4.0f * zz - xx - yy) * u_splatElement.sh[13] +
					SH_C3[5] * z * (xx - yy) * u_splatElement.sh[14] +
					SH_C3[6] * x * (xx - 3.0f * yy) * u_splatElement.sh[15];
			}
		}     
	}

	result += 0.5f;
    result = clamp(result, 0.0, 1.0);
    return result ;
}

mat3 ComputeCov3D(vec4 quat, vec3 scale)
{
    float r = quat.x;
    float x = quat.y;
    float y = quat.z;
    float z = quat.w;
    
    mat3 R = mat3(
        1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - r * z), 2.0 * (x * z + r * y),
        2.0 * (x * y + r * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - r * x),
        2.0 * (x * z - r * y), 2.0 * (y * z + r * x), 1.0 - 2.0 * (x * x + y * y)
    );
    
    mat3 S = mat3(scale.x , 0.0, 0.0 ,
                  0.0, scale.y,  0.0,
                  0.0, 0.0, scale.z
              );
    
    mat3 M = S * R;
    
    return transpose(M) * M;
}

void GetSplatElement(int instanceID){

    int header =  59 * instanceID;
    
    u_splatElement.position.x = texelFetch(u_TBOsplats , header + 0).r;
    u_splatElement.position.y = texelFetch(u_TBOsplats , header + 1).r;
    u_splatElement.position.z = texelFetch(u_TBOsplats , header + 2).r;
    
    // sh
    int shHeader = header + 3 ;
    for(int i = 0 ; i < 16 ; i++) {
        u_splatElement.sh[i] = vec3( texelFetch(u_TBOsplats , shHeader +  0).r ,
                                   texelFetch(u_TBOsplats , shHeader +  1).r ,
                                   texelFetch(u_TBOsplats , shHeader +  2).r );
        shHeader += 3;
    }

    // opacity

    u_splatElement.opacity = texelFetch(u_TBOsplats , header + 51 ).r;
    u_splatElement.opacity = 1.0 / (1.0 + exp(-u_splatElement.opacity));

    // scale
    u_splatElement.scale.r = exp(texelFetch(u_TBOsplats , header + 52).r);
    u_splatElement.scale.g = exp(texelFetch(u_TBOsplats , header + 53).r);
    u_splatElement.scale.b = exp(texelFetch(u_TBOsplats , header + 54).r);

    // rotation propose to be xyzw
    vec4 q;
    q.x =  texelFetch(u_TBOsplats , header + 55).r;
    q.y =  texelFetch(u_TBOsplats , header + 56).r;
    q.z =  texelFetch(u_TBOsplats , header + 57).r;
    q.w =  texelFetch(u_TBOsplats , header + 58).r;
    
    //q.xyz *= mix(-1.0, 1.0, step(0.0, q.w));
    
    u_splatElement.quat = q;
    u_splatElement.quat = normalize(u_splatElement.quat);
  
}

vec4 ComputeSplatProject(vec4 splat_cam, float sizeFactor)
{
    
    vec4 splat_proj = u_renderInfo.projectionMatrix * splat_cam;

    // cull behind camera
    //if (splat_cam.z > 0 ) {
    //     return vec4(0.0, 0.0, 2.0, 1.0);
    //}
    
    mat3 cov_3D = ComputeCov3D(u_splatElement.quat, u_splatElement.scale);
    
    mat3 J_invert_sphere = mat3(1.0);
    if (u_renderInfo.splatInvertSphereEnable == 1.0)
    {
        float R = max(u_renderInfo.splatInvertSphereRaduis, 1e-6);
        float intensity = clamp(u_renderInfo.splatInvertSphereIntensity, 0.0, 1.0);
        float D = max(u_renderInfo.splatInvertSphereDistance, R + 1e-6); 
        float mu = max(u_renderInfo.splatInvertSphereCompression, 0.0) * PERCENT;
        
        vec3 anchorPoint = (u_renderInfo.anchorModelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vec3 sphereCenterAnchor = u_renderInfo.splatInvertSphereCenter.xyz + anchorPoint;
        vec3 positionAnchor = (u_renderInfo.anchorModelMatrix * vec4(u_splatElement.position, 1.0)).xyz;
        vec3 sphereCenter = u_renderInfo.splatInvertSphereCenter.xyz;
        vec3 offset = positionAnchor - sphereCenterAnchor;

        float r = length(offset);

        if (r > 1e-6 && intensity > 1e-6)  
        {
            float denom = 1.0 - (R / D);
            denom = max(abs(denom), 1e-6) * sign(denom);
        
            float r_safe = max(r, 1e-6);
            float v = ((R / r_safe) - (R / D)) / denom;
            float dv_dr = -R / (denom * r_safe * r_safe);
            
            float h = 0.0;
            float dh_dr = 0.0;
            if (abs(mu) < 1e-6)
            {
                h = v;
                dh_dr = dv_dr;
            }
            else
            {
                float arg = 1.0 + mu * v;
                arg = max(arg, 1e-6);
                h = log(arg) / log(1.0 + mu);
                float dh_dv = (mu / arg) / log(1.0 + mu);
                dh_dr = dh_dv * dv_dr;
            }
            
            float f_r = mix(r, R * h, intensity);
            
            f_r = max(f_r, 1e-6);
            
            float s = f_r / r_safe;
            float df_dr = (1.0f - intensity) + intensity * R * dh_dr;
            
            vec3 n = offset / r_safe;
            mat3 nnT = outerProduct(n, n);
            J_invert_sphere = s * mat3(1.0) + (df_dr - s) * nnT;
            J_invert_sphere = mat3(1.0) * (1.0 - intensity) + J_invert_sphere * intensity;

        }
        //cov_3D = ori_cov_3D;
    }
    
    
    mat3 Vrk = mat3(
        cov_3D[0][0], cov_3D[0][1], cov_3D[0][2],
        cov_3D[0][1], cov_3D[1][1], cov_3D[1][2],
        cov_3D[0][2], cov_3D[1][2], cov_3D[2][2]
    );

    float fx = u_renderInfo.focalPixelX;
    float fy = u_renderInfo.focalPixelY;
    float x = splat_cam.x;
    float y = splat_cam.y;
    float z = splat_cam.z;

    mat3 J_project = mat3(
        fx / z, 0.0, -(fx * x) / (z * z),
        0.0, fy / z, -(fy * y) / (z * z),
        0.0, 0.0, 0.0
    );
    J_project = transpose(J_project);

    // 3.  T =  local_space -> screen_space
    mat3 T = J_project * mat3(u_renderInfo.viewMatrix * u_renderInfo.transformModelMatrix) * J_invert_sphere * mat3(u_renderInfo.anchorModelMatrix);

    // 4. cov = M * Vrk * M^T
    mat3 cov_2D = T * Vrk * transpose(T);

    float diagonal1   = cov_2D[0][0] +0.3;
    float offDiagonal = cov_2D[0][1];
    float diagonal2   = cov_2D[1][1] +0.3;

    float mid = 0.5 * (diagonal1 + diagonal2);
    float radius = length(vec2((diagonal1 - diagonal2) / 2.0, offDiagonal));
    float lambda1 = mid + radius;
    float lambda2 = max(mid - radius, 0.1);
    vec2 diagonalVector = normalize(vec2(offDiagonal, lambda1 - diagonal1));
    vec2 v1 = min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;
    vec2 v2 = min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);


     // early out tiny splats
     //TODO: figure out length units and expose as uniform parameter
     //TODO: perhaps make this a shader compile-time option
     //if (dot(v1, v1) < 4.0 && dot(v2, v2) < 4.0) {
     //    return vec4(0.0, 0.0, 2.0, 1.0);
     //}

    texCoord = aPos.xy * 4.0;
    
    
    float r1 = length(v1);
    float r2 = length(v2);
    
    float minSplatRadius = 1.0; // pixel
    float maxAspectRatio = 10.0;
    
    float majorR = max(r1, r2);
    float minorR = max(min(r1, r2), 1e-6);
    float aspectRatio = majorR / minorR;
    if (r1 < minSplatRadius || r2 < minSplatRadius || aspectRatio > maxAspectRatio)
    {
        return vec4(0.0, 0.0, 2.0, 1.0);
    }

    float minR1 = 3.0 * step(0.000001, sizeFactor);
    float minR2 = 3.0 * step(0.000001, sizeFactor);

    float finalR1 = mix(minR1, r1, sizeFactor);
    float finalR2 = mix(minR2, r2, sizeFactor);

    v1 = normalize(v1) * finalR1;
    v2 = normalize(v2) * finalR2;
    
    vec2 screenOffset = (texCoord.x * v1 + texCoord.y * v2) / u_renderInfo.viewport * splat_proj.w;
    
    
    splat_proj.xy += screenOffset;

    return splat_proj;
}
