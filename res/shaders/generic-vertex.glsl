attribute vec2 pos;
attribute vec2 tex;
attribute vec4 color;

varying vec2 tex_coord;
varying vec4 frag_color;

uniform mat4 ortho;

void main() {
	gl_Position = ortho * vec4(pos.xy, 1.0, 1.0);
	tex_coord = tex;
	frag_color = color;
}
