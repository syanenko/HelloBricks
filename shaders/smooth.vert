#version 120
//
//  Text rendering
//
attribute vec3 a_position;
attribute vec3 a_bc;

varying vec3 v_position;
varying vec3 v_bc;

uniform mat4 u_projection;
uniform mat4 u_modelview;

void main (void)
{
  v_bc = a_bc;
  v_position = a_position;

  gl_Position = u_projection * u_modelview * vec4( a_position, 1.0 );
}
