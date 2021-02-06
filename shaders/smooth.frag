#version 120
//
//  Text rendering
//
varying vec3 v_position;
varying vec3 v_bc;

void main(void)
{ 
  if( pow((v_bc.x / 2 + v_bc.z), 2) < v_bc.z)
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
  else
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
