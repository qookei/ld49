#include <window.hpp>
#include <iostream>

#include <glm/glm.hpp>
//#include <glm/ext/matrix_clip_space.hpp>

#include <gl/shader.hpp>
#include <gl/mesh.hpp>
#include <gl/texture.hpp>

#include <text.hpp>

struct scene {
	scene()
	: fnt_{"res/font.txt"}, prog_{
		gl::shader{GL_VERTEX_SHADER, "res/shaders/generic-vertex.glsl"},
		gl::shader{GL_FRAGMENT_SHADER, "res/shaders/generic-fragment.glsl"}
	} {
		gl::vertex vtx[] = {
			{{0, 0}, {0, 0}, {1, 1, 1, 1}},
			{{128, 0}, {1, 0}, {1, 1, 1, 1}},
			{{128, 256}, {1, 1}, {1, 1, 1, 1}},

			{{0, 0}, {0, 0}, {1, 1, 1, 1}},
			{{128, 256}, {1, 1}, {1, 1, 1, 1}},
			{{0, 256}, {0, 1}, {1, 1, 1, 1}}
		};
		mesh_.vbo().store_regenerate(vtx, sizeof(vtx), GL_STATIC_DRAW);
		tex_.load("res/font.png");
	}

	void tick(double delta, const input_state &input) {
	}

	void render() {
		text t{&fnt_};
		t.set_text("hello world!\nthis is a test\nthe quick brown fox jumps over the lazy dog\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
		prog_.use();
		prog_.set_uniform("ortho", ortho);
		mesh_.render();
		t.render();
	}

private:
	font fnt_;

	gl::texture2d tex_;
	gl::mesh mesh_;
	gl::program prog_;

	glm::mat4 ortho = glm::ortho(0.f, static_cast<float>(window::width),
			static_cast<float>(window::height), 0.f);
};

int main() {
	std::cout << "Hello world!\n";

	window wnd;

	scene s_{};

	wnd.attach_ticker(s_);
	wnd.attach_renderer(s_);
	wnd.enter_main_loop();
}
