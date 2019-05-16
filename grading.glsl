#version 410
uniform vec2 uResolution;
uniform sampler2D uImages[1];
out vec4 outColor;

#define sat(x) clamp(x,0.,1.)

/// Color conversions ///
// https://gist.github.com/sugi-cho/6a01cae436acddd72bdf
vec3 rgb2hsv(vec3 c)
{
    vec4 K=vec4(0,-1/3.,2/3.,-1),
    p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g)),
    q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
    float d=q.x-min(q.w,q.y),e=1.0e-10;
    return vec3(abs(q.z+(q.w-q.y)/(6*d+e)),d/(q.x+e),q.x);
}
vec3 hsv2rgb(vec3 c){return c.z*mix(vec3(1),sat(abs(fract(vec3(1,2/3.,1/3.)+c.x)*6-3)-1),c.y);}
vec3 rgb2hsv(float r,float g,float b){return rgb2hsv(vec3(r,g,b));}
vec3 hsv2rgb(float h,float s,float v){return hsv2rgb(vec3(h,s,v));}

// from http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
vec3 colorFromKelvin(float temperature) // photographic temperature values are between 15 to 150
{
    float r, g, b;
    if(temperature <= 66.0)
    {
        r = 1.0;
        g = sat((99.4708025861 * log(temperature) - 161.1195681661) / 255.0);
        if(temperature < 19.0)
            b = 0.0;
        else
            b = sat((138.5177312231 * log(temperature - 10.0) - 305.0447927307) / 255.0);
    }
    else
    {
        r = sat((329.698727446 / 255.0) * pow(temperature - 60.0, -0.1332047592));
        g = sat((288.1221695283  / 255.0) * pow(temperature - 60.0, -0.0755148492));
        b = 1.0;
    }
    return vec3(r, g, b);
}

uniform vec3 uLift = vec3(0.0);
uniform vec3 uGamma = vec3(0.0);
uniform vec3 uGain = vec3(0.0);
uniform vec3 uOffset = vec3(0.0);

uniform float uContrast = 1.0;
uniform float uContrastPivot = 0.5;

uniform float uSaturation = 1.0;
uniform float uHue = 0.0;
uniform float uTemperature = 66.0;
uniform float uUnsharpMask = 0.0;

float Luma(vec3 color) { return dot(color, vec3(0.2126, 0.7152, 0.0722)); }

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm( vec3 x )
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return sat((x*(a*x+b))/(x*(c*x+d)+e));
}

// Good & fast sRgb approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html
vec3 LinearToSRGB(vec3 rgb)
{
    rgb=max(rgb,vec3(0,0,0));
    return max(1.055*pow(rgb,vec3(0.416666667))-0.055,0.0);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / uResolution;
	ivec2 texel = ivec2(gl_FragCoord.xy);
	vec3 v = texture(uImages[0], uv).xyz;
	
	// unsharp mask
	vec4 blurry = 0.25 * (texture(uImages[0], vec2(texel - ivec2(1, 0)) / uResolution) 
					    + texture(uImages[0], vec2(texel - ivec2(0, 1)) / uResolution)
						+ texture(uImages[0], vec2(texel + ivec2(1, 0)) / uResolution) 
						+ texture(uImages[0], vec2(texel + ivec2(0, 1)) / uResolution));
	v += (v - blurry.xyz) * uUnsharpMask;
	v = sat(v);

	// contrast
	// contrast below 1, just fades from the pivot to the color
	v = mix(vec3(uContrastPivot), v, sat(uContrast));
	
	vec3 p = vec3(1.0 / sat(2.0 - uContrast));
	vec3 dark = pow(v / uContrastPivot, p) * uContrastPivot;
	vec3 ip = vec3(1.0 - uContrastPivot);
	vec3 light = 1.0 - pow(1.0 / ip - v / ip, p) * ip;
	v = mix(dark, light, greaterThan(v, vec3(uContrastPivot)));
	
	// saturation
	float luma = Luma(v);
	v = mix(vec3(luma), v, uSaturation);
	
	// hue shift
	v = hsv2rgb(rgb2hsv(v) + vec3(fract(uHue / 6.0), 0.0, 0.0));
	
	// assuming luma didnt change since saturation adjustment
	// v = mix(v, hsv2rgb(vec3(rgb2hsv(colorFromKelvin(uTemperature)).xy, luma)), 1.0);
	v *= vec3(1.0) / colorFromKelvin(uTemperature);

	// three way color corrector
	v = pow(max(vec3(0.0), v * (1.0 + uGain - uLift) + uLift + uOffset), max(vec3(0.0), 1.0 - uGamma));
	
	// convert to gamma space
    v = LinearToSRGB(v); // ACESFilm(v * uExposure)

	outColor = vec4(v, 1.0);
}
