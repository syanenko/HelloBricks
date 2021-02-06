#version 120
//
//  Text rendering
//
uniform sampler2D u_texture;
uniform vec2      u_resolution;

void main(void)
{ 
  vec2 position = gl_FragCoord.xy / u_resolution.xy;
  vec4 color = texture2D(u_texture, position);

  if (mod(color.x * 255.0, 2.0) == 0.0)
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  else
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
