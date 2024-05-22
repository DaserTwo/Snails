#version 330

in vec3 fragPosition;

in vec2 fragTexCoord;
in vec4 fragColor;

const int count = 607;

uniform vec4 colors[count];
uniform vec2 positions[count];

out vec4 finalColor;

void main()
{
    vec2 coord = fragPosition.xy;
    coord.y = -coord.y;
    int i = 0;
    vec4 color = colors[0]; 
    float dist = distance((positions[0] * 2) - 1, coord);
    while (i < count){
        float d = distance((positions[i] * 2) - 1, coord);
        if (dist > d){
            color = colors[i];
            dist = d;
        }

        i++;
    }

    finalColor = color;
}
