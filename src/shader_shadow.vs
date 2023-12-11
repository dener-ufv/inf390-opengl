#version 330
in vec3 PositionShadow;
uniform mat4 MVPMatrixShadow;

void main()
{
    gl_Position = MVPMatrixShadow * vec4(PositionShadow, 1.0);
}