#version 120
//
//  Text rendering
//
attribute vec3 a_position;

uniform mat4 u_projection;
uniform mat4 u_modelview;

void main (void)
{
  gl_Position = u_projection * u_modelview * vec4( a_position, 1.0 );
}
