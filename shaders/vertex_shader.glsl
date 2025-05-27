#version 330 core

uniform mat4 uProjection;
uniform mat4 uModel;
uniform mat4 uView;
layout(location = 0) in vec3 aPos;     // vertex position
layout(location = 1) in vec3 aColor;   // vertex color

out vec3 vertexColor;  // pass to fragment shader

void main() {
   
    gl_Position = uProjection *uView* uModel * vec4(aPos, 1.0);
    vertexColor = aColor;
    
}