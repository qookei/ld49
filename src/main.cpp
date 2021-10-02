#include <window.hpp>
#include <iostream>

#include <glm/glm.hpp>

#include <gl/shader.hpp>
#include <gl/mesh.hpp>
#include <gl/texture.hpp>

#include <sprite.hpp>
#include <text.hpp>
#include <time.hpp>

#include <random>

std::mt19937 global_mt;

template <int N>
struct clouds {
	clouds(gl::program &prog, time_tracker &tt)
	: spr_{prog, "res/cloud.png", 64, 64},
			alarm_{tt.add_alarm(0.07)} {
		for (auto &c : pos_) {
			c.x = x_dist_(global_mt);
			c.y = y_dist_(global_mt);
		}
	}

	void tick() {
		if (alarm_.expired()) {
			alarm_.rearm();
			for (auto &c : pos_) {
				c.x += 1;

				if (c.x >= window::width) {
					c.x = -64;
					c.y = y_dist_(global_mt);
				}
			}
		}
	}

	void render() {
		for (auto &c : pos_) {
			spr_.x = c.x;
			spr_.y = c.y;
			spr_.render();
		}
	}
private:
	std::array<glm::vec2, N> pos_{};
	sprite spr_;
	alarm &alarm_;
	std::uniform_int_distribution<int> x_dist_{0, window::width};
	std::uniform_int_distribution<int> y_dist_{0, window::height - 32};
};

struct scene {
	void tick(double delta, const input_state &input) {
		time_tracker_.tick(delta);
		clouds_.tick();
	}

	void render() {
		prog_.set_uniform("ortho", ortho);

		clouds_.render();

		text t{prog_, fnt_};
		t.set_text("hello world!\nthis is a test\nthe quick brown fox jumps over the lazy dog\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
		t.render();
	}

private:
	gl::program prog_{
		gl::shader{GL_VERTEX_SHADER, "res/shaders/generic-vertex.glsl"},
		gl::shader{GL_FRAGMENT_SHADER, "res/shaders/generic-fragment.glsl"}
	};

	font fnt_{"res/font.txt"};

	time_tracker time_tracker_;

	clouds<16> clouds_{prog_, time_tracker_};

	glm::mat4 ortho = glm::ortho(0.f, static_cast<float>(window::width),
			static_cast<float>(window::height), 0.f);
};

int main() {
	std::random_device dev{};
	global_mt = std::mt19937{dev()};

	window wnd;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scene s_{};

	wnd.attach_ticker(s_);
	wnd.attach_renderer(s_);
	wnd.enter_main_loop();
}
