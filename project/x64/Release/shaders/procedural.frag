#version 330 core
in vec3 f_Col;
in vec3 Normal;
in vec2 texCoord;
in vec3 WorldPos;
in float distFromPos;
in float dispFactor;
in vec3 wireframeColor;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 gEyeWorldPos;
uniform vec3 u_ViewPosition;
uniform vec3 fogColor;
uniform vec3 seed;
uniform vec2 offset;
uniform int octaves;
uniform bool drawFog;
uniform float gDispFactor;
uniform float freq;
uniform bool normals;
uniform float u_grassCoverage;

uniform sampler2D grass, rock;
uniform sampler2D randomTexture;

out vec4 FragColor;


//float random generator from a 2-dim vec
float rand2d(in vec2 st)
{
	return fract(dot(texture(randomTexture, st/3000).rg, vec2(12.9898, 78.233) + seed.xy));
}

//used for central difference derivatives computing

float InterpolatedNoise(vec2 xy) {
	vec2 coords = floor(xy);
	float a = rand2d(coords);
	float b = rand2d(coords + vec2(1.0, 0.0));
	float c = rand2d(coords + vec2(0.0, 1.0));
	float d = rand2d(coords + vec2(1.0, 1.0));

	vec2 w = fract(xy);
	w = w*w*w*(10.0 + w*(-15.0 + 6.0*w));

	float k0 = a, 
	k1 = b - a, 
	k2 = c - a, 
	k3 = d - c - b + a;

	return k0 + k1*w.x + k2*w.y + k3*w.x*w.y;

}

const mat2 m = mat2(0.8,-0.6,0.6,0.8);

//simplex noise 
vec3 InterpolatedNoiseD(float x, float y) {
	vec2 xy = vec2(x,y);
	vec2 randomInput = floor(xy);
	float a = rand2d(randomInput);
	float b = rand2d(randomInput + vec2(1.0, 0.0));
	float c = rand2d(randomInput + vec2(0.0, 1.0));
	float d = rand2d(randomInput + vec2(1.0, 1.0));

	vec2 fracW = fract(xy);
	vec2 w = fracW;
	w = w*w*w*(10.0 + w*(-15.0 + 6.0*w));
	vec2 dw = 30*fracW*fracW*(fracW*fracW - 2*fracW + 1);
	
	float k0 = a, 
	k1 = b - a, 
	k2 = c - a, 
	k3 = d - c - b + a;

	float n = k0 + k1*w.x + k2*w.y + k3*w.x*w.y;
	float dndx = (k1 + k3*w.y)*dw.x;
	float dndy = (k2 + k3*w.x)*dw.y;

	return vec3(n, dndx, dndy);
}
uniform float power;

//used for central difference derivatives computing
float derivativeFBM(float x, float y){
    vec2 st = vec2(x,y);
    //st *= freq;
    
    //int numOctaves = 10;
	float persistence = 0.5;
	float total = 0.0,
		frequency = 0.005*freq,
		amplitude = gDispFactor;
	for (int i = 0; i < min(9, octaves); ++i) {
		frequency *= 2.0;
		amplitude *= persistence;

        //st = frequency*m*st;

		vec2 v = frequency*m*st;
		vec3 n = InterpolatedNoiseD(v.x,v.y);
		vec2 d = vec2(n.y, n.z);
		total += n.x * amplitude /(1.0 + dot(d,d));
	}
	return pow(total, power);
}


vec3 computeNormals(vec3 WorldPos){
	//step length
	float st = 1.0;
	float dhdu = (derivativeFBM((WorldPos.x + st), WorldPos.z) - derivativeFBM((WorldPos.x - st), WorldPos.z))/(2.0*st);
	float dhdv = (derivativeFBM( WorldPos.x, (WorldPos.z + st)) - derivativeFBM(WorldPos.x, (WorldPos.z - st)))/(2.0*st);

	vec3 X = vec3(1.0, dhdu, 1.0);
	vec3 Z = vec3(0.0, dhdv, 1.0);

	vec3 n = normalize(cross(Z,X));
	n = mix(n, vec3(0.0,1.0,0.0), 0.5);
	return n;
}

vec3 ambient(){
	float ambientStrength = 0.65; 
    vec3 ambient = ambientStrength * u_LightColor; 
    return ambient;
}

vec3 computeNormals(vec2 gradient){
	vec3 X = vec3(1.0, gradient.r, 0.0);
	vec3 Z = vec3(0.0, gradient.g, 1.0);

	vec3 n = normalize(cross(Z,X));

	return n;
}

//diffuse color
vec3 diffuse(vec3 normal){
	vec3 lightDir = normalize(u_LightPosition - WorldPos);
	float diffuseFactor = max(0.0, dot(lightDir, normal));
	const float diffuseConst = 0.45;
	vec3 diffuse = diffuseFactor * u_LightColor * diffuseConst;
	return diffuse;
}

//simplex noise fbm
float simplexFBM(float x, float y, int oct){
	
	vec2 distances = vec2(500.0, 2000.0);
	int distanceFactor = int(clamp( (distances.y - distFromPos)*3.0 / (distances.y - distances.x), 0.0, 3.0));
	distanceFactor = 3 - distanceFactor;

    int numOctaves = oct;
	float persistence = 0.5;
	float total = 0,
		frequency = 0.05*freq,
		amplitude = 1.0;
	for (int i = 0; i < numOctaves; ++i) {
		frequency *= 2;
		amplitude *= persistence;
		
		total += InterpolatedNoise(vec2(x,y)*frequency) * amplitude;
	}
	return total;
}


uniform vec3 rockColor;
uniform float fogFalloff;

const float c = 18.;
const float b = 3.e-6;

float applyFog( in vec3  rgb,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  cameraPos,   // camera position
               in vec3  rayDir )  // camera to point vector
{
    float fogAmount = c * exp(-cameraPos.y*fogFalloff) * (1.0-exp( -dist*rayDir.y*fogFalloff ))/rayDir.y;
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return clamp(fogAmount,0.0,1.0);
}


void main()
{
	// calculate fog color 
	vec2 u_FogDist = vec2(2500.0, 10000.0);
	bool normals_fog = true;
	float fogFactor = applyFog(vec3(0.0), distance(gEyeWorldPos, WorldPos), gEyeWorldPos, normalize(WorldPos - gEyeWorldPos));
	float eps = 0.1;
	if(fogFactor >= 0.0 && fogFactor > 1. - eps){
		normals_fog = false;
	}
	
	vec3 n;
	if(normals && normals_fog){
		n = computeNormals(WorldPos);
		n = normalize(n);
	}else{
		n = vec3(0,1,0);
	}


	float grassBlendingCoeff = clamp(simplexFBM(WorldPos.x, WorldPos.z, 2)*2.0 - 0.2, 0.0, 1.0);
	vec4 grassColor = texture(grass, texCoord*5.0);
	vec4 rockColor = texture(rock, texCoord*5.0)*vec4(1.0, 0.9, 1.0, 1.0);

	vec4 terrainColor = mix(grassColor, rockColor, grassBlendingCoeff);

	vec3 ambient = ambient();
	vec3 diffuse = diffuse(n);

	// putting all together
    vec4 color = terrainColor*vec4((ambient + diffuse)*vec3(1.0f) , 1.0f);
	if(drawFog){
		FragColor = mix(color, vec4(mix(fogColor*1.1,fogColor*0.85,clamp(WorldPos.y/(1500.*16.)*gDispFactor,0.0,1.0)), 1.0f), fogFactor);
	}else{
		FragColor = vec4(wireframeColor, 1.0);
	}
	FragColor.a = 1.0;
};