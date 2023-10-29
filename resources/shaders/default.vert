
#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec2 uvCoords;
out vec2 uvCoordsOut;

void main()
{
    uvCoordsOut = uvCoords;
    gl_Position = vec4(aPos, 1.0);
}