#version 330 core
out vec4 FragColor;
in vec2 uvCoordsOut;

uniform sampler2D uSampler;

void main()
{
    FragColor = texture(uSampler, uvCoordsOut);
}