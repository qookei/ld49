precision mediump float;
varying vec2 tex_coord;

uniform sampler2D tex_sampler;
uniform vec4 obj_color;

void main() {
	gl_FragColor = texture2D(tex_sampler, tex_coord) * obj_color;
}
