#include <window.hpp>
#include <iostream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

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
	std::uniform_int_distribution<int> y_dist_{0, window::height - 16};
};

struct particles {
	particles(gl::program &prog)
	: spr_{prog, "res/particle.png", 2, 2} { }

private:
	struct particle {
		double x, y;
		double xvel, yvel;
		int xdir;
	};

public:
	void add_particle(double x, double y) {
		std::uniform_real_distribution<double> y_dist{-1, -4};
		std::uniform_real_distribution<double> x_dist{0, 3};
		std::uniform_int_distribution<int> dist{0, 1};
		parts_.push_back({x, y, x_dist(global_mt) * 50, y_dist(global_mt) * 50, dist(global_mt) * 200 ? 1 : -1});
	}

	void tick(double delta) {
		for (auto it = parts_.begin(); it != parts_.end();) {
			auto &p = *it;

			p.x += p.xvel * p.xdir * delta;

			if (p.xvel > 0)
				p.xvel -= 20;
			else
				p.xvel = 0;

			p.y += p.yvel * delta;
			p.yvel += 90;

			if (p.y >= window::height)
				it = parts_.erase(it);
			else
				++it;
		}
	}

	void render() {
		for (auto &p : parts_) {
			spr_.x = p.x;
			spr_.y = p.y;
			spr_.render();
		}
	}

private:
	sprite spr_;

	std::vector<particle> parts_;
};

struct blocks {
	blocks(gl::program &prog, particles &part)
	: prog_{prog}, part_{part} { }

private:
	struct block {
		block(gl::program &prog, particles &part, int frame)
		: spr_{prog, "res/blocks.png", 8, 8, frame}, part_{part} {
			std::uniform_real_distribution<double> dist{8., 16.};
			time_left_ = dist(global_mt);
		}

		void tick(double delta) {
			switch (state_) {
				case state::solid:
					time_left_ -= delta;
					if (time_left_ <= 0)
						state_ = state::shaking;
					break;
				case state::shaking:
					time_shake_ -= delta;
					if (time_shake_ <= 0)
						state_ = state::falling;
					time_particle_ -= delta;
					if (time_particle_ <= 0) {
						for (int i = 0; i < 8; i++)
							part_.add_particle(x + 4, y + 8);
						time_particle_ = 0.05;
					}
					break;
				case state::falling:
					y += yvel * delta;
					yvel += 10;
					if (y >= window::height)
						should_be_removed_ = true;
					break;
			}
		}

		void render() {
			int offx = 0, offy = 0;

			if (state_ == state::shaking) {
				std::uniform_int_distribution<int> dist{-1, 1};
				offx = dist(global_mt);
				offy = dist(global_mt);
			}

			spr_.x = x + offx;
			spr_.y = y + offy;
			spr_.render();
		}

		enum class state {
			solid, shaking, falling
		} state_ = state::solid;

		bool check_collision(double ox, double oy, double ow, double oh) {
			return state_ != state::falling && !(ox > (x + 8)
				|| (ox + ow) < x
				|| (oy + oh) < y
				|| oy > (y + 8));
		}

		sprite spr_;
		particles &part_;
		double time_left_;
		double time_shake_ = 0.2;
		double time_particle_ = 0;
		bool should_be_removed_ = false;
		double x = 0, y = 0;
		double yvel = 0;
	};

public:
	void tick(double delta) {
		for (auto it = blocks_.begin(); it != blocks_.end();) {
			auto &[_, bl] = *it;
			bl.tick(delta);
			if (bl.should_be_removed_)
				it = blocks_.erase(it);
			else
				++it;
		}
	}

	void add_platform() {
		while (true) {
			auto len = l_dist_(global_mt);

			std::uniform_int_distribution<int> x_dist_{2, window::width / 8 - len - 1};
			std::uniform_int_distribution<int> y_dist_{10, window::height / 24 - 3};
			int xx = x_dist_(global_mt);
			int yy = x_dist_(global_mt);

			bool ok = true;
			for (int i = 0; i < len; i++) {
				if (blocks_.contains(glm::ivec2{xx + i, yy})) {
					ok = false;
					break;
				}
			}

			if (!ok) continue;

			for (int i = 0; i < len; i++) {
				block b{prog_, part_, f_dist_(global_mt)};
				b.x = (xx + i) * 8;
				b.y = yy * 8;
				blocks_.emplace(glm::ivec2{xx + i, yy}, std::move(b));
			}

			break;
		}
	}

	void render() {
		for (auto &[_, bl] : blocks_)
			bl.render();
	}

	bool check_collision(double x, double y, double w, double h) {
		for (auto &[_, bl] : blocks_) {
			if (bl.check_collision(x, y, w, h))
				return true;
		}

		return false;
	}

private:
	gl::program &prog_;
	particles &part_;
	std::unordered_map<glm::ivec2, block> blocks_;
	std::uniform_int_distribution<int> l_dist_{2, 8};
	std::uniform_int_distribution<int> f_dist_{0, 23};
};

struct player {
	player(blocks &blocks, gl::program &prog)
	: blocks_{blocks}, spr_{prog, "res/player.png", 8, 8} { }

public:
	void tick(double delta, input_state &input) {
		if (input.down_keys[SDLK_LEFT]) {
			xdir = -1;
			xvel = 130;
		}

		if (input.down_keys[SDLK_RIGHT]) {
			xdir = 1;
			xvel = 130;
		}

		if (input.just_pressed_keys.contains(SDLK_UP) && jump_ctr) {
			yvel = -240;
			jump_ctr--;
		}

		constexpr double steps = 50;
		for (int i = 0; i < steps; i++) {
			auto newx = x + (xvel * xdir * delta) / steps;
			auto newy = y + (yvel * delta) / steps;

			if (!blocks_.check_collision(x, newy, 7, 7)) {
				y = newy;
			} else {
				if (yvel > 0)
					jump_ctr = 2;
				yvel = 0;
			}

			if (!blocks_.check_collision(newx, y, 7, 7)) {
				x = newx;
			} else {
				xvel = 0;
			}
		}

		if (xvel > 0) {
			xvel -= 10;
		} else {
			xvel = 0;
			xdir = 0;
		}

		yvel += 10;

		if (y >= (window::height - 8)) {
			y = window::height - 8;
			yvel = 0;
			jump_ctr = 2;
		}
	}

	void render() {
		spr_.x = x;
		spr_.y = y;
		spr_.render();
	}

private:
	blocks &blocks_;
	sprite spr_;
	double x = 0, y = 0;
	double xvel = 0, yvel = 0;
	int xdir = 0;
	int jump_ctr = 2;
};

struct scene {
	void tick(double delta, input_state &input) {
		time_tracker_.tick(delta);
		clouds_.tick();

		if (input.just_pressed_keys.contains(SDLK_SPACE))
			blocks_.add_platform();

		blocks_.tick(delta);
		particles_.tick(delta);
		player_.tick(delta, input);
	}

	void render() {
		prog_.set_uniform("ortho", ortho);

		clouds_.render();

		bg_.render();

		blocks_.render();
		player_.render();
		particles_.render();

		//text t{prog_, fnt_};
		//t.x = 38;
		//t.y = 72;
		//t.set_text(" hello world!\nthis is a test");
		//t.render();
	}

private:
	gl::program prog_{
		gl::shader{GL_VERTEX_SHADER, "res/shaders/generic-vertex.glsl"},
		gl::shader{GL_FRAGMENT_SHADER, "res/shaders/generic-fragment.glsl"}
	};

	font fnt_{"res/font.txt"};

	time_tracker time_tracker_;

	clouds<20> clouds_{prog_, time_tracker_};

	particles particles_{prog_};
	blocks blocks_{prog_, particles_};

	player player_{blocks_, prog_};

	sprite bg_{prog_, "res/bg.png", 160, 120};

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
