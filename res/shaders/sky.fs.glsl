#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

float gradientNoise(in vec2 uv) {
    return fract(52.9829189 * dot(uv, vec2(0.06711056, 0.00583715)));
}
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
//    float x = 0.0;
//
//    if (1.0 - fragTexCoord.y > 0.5)
//        x = 1.0;

//    finalColor = vec4(vec3(rand(fragTexCoord.xy)), 1.0);
//    finalColor = vec4(x, 0.0, 0.0, 1.0);
    finalColor = vec4((mix(vec3(0.225, 0.415, 0.631), vec3(0.327, 0.594, 0.860), .7 - 1.0 + fragNormal.y) * 1.) + (rand(fragTexCoord.xy) / 128.), 1.0);
//    finalColor = vec4(fragNormal.y, 0.0, 0.0, 1.0);
}
