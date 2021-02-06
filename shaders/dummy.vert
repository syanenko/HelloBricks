#version 120
//
//  Text rendering
//
uniform mat4 u_projection;
uniform mat4 u_modelview;

attribute vec3 a_position;
varying vec4 vColor;

void main()
{
  vColor = gl_Color;
  gl_Position = u_projection * u_modelview * vec4( a_position, 1.0 );
}
