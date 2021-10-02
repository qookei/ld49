#include <window.hpp>
#include <iostream>

#include <glm/glm.hpp>

#include <gl/shader.hpp>
#include <gl/mesh.hpp>
#include <gl/texture.hpp>

#include <sprite.hpp>
#include <text.hpp>
#include <time.hpp>

struct scene {
	void tick(double delta, const input_state &input) {
		time_tracker_.tick(delta);

		if (al.expired()) {
			i = (i + 1) % 4;
			s_.set_frame(i);
			al.rearm();
		}
	}

	void render() {
		prog_.set_uniform("ortho", ortho);

		text t{prog_, fnt_};
		t.set_text("hello world!\nthis is a test\nthe quick brown fox jumps over the lazy dog\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
		t.render();

		s_.render();
	}

private:
	gl::program prog_{
		gl::shader{GL_VERTEX_SHADER, "res/shaders/generic-vertex.glsl"},
		gl::shader{GL_FRAGMENT_SHADER, "res/shaders/generic-fragment.glsl"}
	};

	font fnt_{"res/font.txt"};

	sprite s_{prog_, "res/test.png", 16, 16};
	int i = 0;

	time_tracker time_tracker_;
	alarm &al = time_tracker_.add_alarm(0.5);

	glm::mat4 ortho = glm::ortho(0.f, static_cast<float>(window::width),
			static_cast<float>(window::height), 0.f);
};

int main() {
	window wnd;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scene s_{};

	wnd.attach_ticker(s_);
	wnd.attach_renderer(s_);
	wnd.enter_main_loop();
}
