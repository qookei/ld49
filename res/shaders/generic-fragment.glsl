precision mediump float;
varying vec4 frag_color;
varying vec2 tex_coord;

uniform sampler2D tex;

void main() {
	gl_FragColor = texture2D(tex, tex_coord) * frag_color;
}
