attribute vec2 pos;
attribute vec2 tex;

varying vec2 tex_coord;

uniform vec2 obj_pos;
uniform mat4 ortho;

void main() {
	gl_Position = ortho * vec4(pos + obj_pos, 1.0, 1.0);
	tex_coord = tex;
}
