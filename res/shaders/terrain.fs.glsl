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

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);

    float flatShading = abs(fragNormal.y) * 1.0 +
        abs(fragNormal.x) * 0.8 +
        abs(fragNormal.z) * 0.6;

    finalColor = vec4(texelColor.rgb * fragColor.rgb * flatShading, 1.0);
}
