/** Atmosphere Rendering
* - ref: <https://zhuanlan.zhihu.com/p/595576594>
*/

#![version("430 core")]

#![vertex]
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

// World Space Position
out vec3 vPositionWS;
// Projection Space Position
out vec4 vPositionPS;

void main() {
    vec4 position = vec4(aPosition, 1.0);
    
    position = model * position;
    vPositionWS = position.xyz;
    
    position = projection * view * position;
    vPositionPS = position;

    gl_Position = position;
}

#![fragment]
#![use("common/atmos.su")]
#![const<int>("SCATTERING_SAMPLE_NUM")]

out vec4 FragColor;

in vec3 vPositionWS;
in vec4 vPositionPS;

uniform vec3 sunPosition;
uniform vec3 earthPosition;
uniform vec3 cameraPosition;
uniform vec3 sunLuminance;
uniform sampler2D textureBackGround;
uniform sampler2D textureTransmittanceLUT;

const float earthRadius = ATMOS_PLANET_RADIUS;
const float atmosRadius = ATMOS_ATMOS_RADIUS;

bool IntersectSphere(vec3 center, float radius, vec3 origin, vec3 direction, out float tmin, out float tmax) {
    vec3 oc = origin - center;
    float b = dot(oc, direction);
    float c = dot(oc, oc) - radius * radius;
    
    const float EPS = 1e-6;
    float disc = b * b - c;
    if (disc < -EPS) {
        tmin = -1.0;
        tmax = -1.0;
        return false;
    }
    disc = max(disc, 0.0);

    float s = sqrt(disc);
    tmin = -b - s;
    tmax = -b + s;
    return true;
}

/// Get transmittance value from LUT.
/// - `p1`: the point inside atmosphere.
/// - `p2`: the point on atmosphere.
vec3 GetTransmittanceFromLUT(vec3 p1, vec3 p2) {
    float r  = distance(earthPosition, p1);
    float mu = dot(normalize(p1 - earthPosition), normalize(p2 - p1));

    vec2 uv = TransmittanceParamsToUV(mu, r);
    return texture(textureTransmittanceLUT, uv).rgb;
}

/// Transmittance of sunlight, considering the occlusion of planet itself.
/// - `p`: position of scattering point
/// - `dir`: direction from `p` to sunPos (normalized)
vec3 TransmittanceToSun(vec3 p, vec3 dir) {
    float tmin = 0.0;
    float tmax = 0.0;
    if (IntersectSphere(earthPosition, earthRadius, p, dir, tmin, tmax)) {
        if (tmax > 0.0) {
            return vec3(0.0);
        }
    }
    
    // Get intersect point on atmosphere sphere
    IntersectSphere(earthPosition, atmosRadius, p, dir, tmin, tmax);
    vec3 pA = p + dir * tmax;

    return GetTransmittanceFromLUT(p, pA);
}

void main() {
    vec3 viewPos  = cameraPosition;
    vec3 viewDir  = normalize(vPositionWS - viewPos);
    vec3 lightDir = normalize(earthPosition - sunPosition);
    
    float tmin = 0.0;
    float tmax = 0.0;
    float innerT = -1.0;
    if (IntersectSphere(earthPosition, earthRadius, viewPos, viewDir, tmin, tmax)) {
        // Hit planet's surface, used as `end` position.
        innerT = tmin;
    }

    IntersectSphere(earthPosition, atmosRadius, viewPos, viewDir, tmin, tmax);
    
    // Safety: Fragment on atmosphere mesh always hits.
    float outerT = tmin;

    // If innerT miss, means `end` point on atmosphere.
    if (innerT < 0.0) {
        innerT = tmax;
    }

    // Integral's deltaS, startPos, endPos.
    float ds = (innerT - outerT) / float(SCATTERING_SAMPLE_NUM);
    vec3 start  = viewPos + viewDir * outerT;
    vec3 end    = viewPos + viewDir * innerT;
    
    vec3 p1 = start + (viewDir * ds) * 0.2;
    vec3 opticalDepthT2 = vec3(0.0);
    vec3 color = vec3(0.0);
    
    for (int i = 0; i < SCATTERING_SAMPLE_NUM; ++i) {
        vec3 t1 = TransmittanceToSun(p1, -lightDir);
        vec3 s  = Scattering(earthPosition, p1, lightDir, -viewDir);
        vec3 t2 = GetTransmittanceFromLUT(p1, start);

        color += t1 * s * t2 * ds * sunLuminance;
        p1 += viewDir * ds;
    }

    // Compute color contributed by earth surface.
    vec2 texCoord = (vPositionPS.xy / vPositionPS.w) * 0.5 + 0.5;
    vec3 background = texture(textureBackGround, texCoord).rgb;
    vec3 transmittance = GetTransmittanceFromLUT(end, start);
    
    FragColor = vec4(color + background * transmittance, 1.0);
}