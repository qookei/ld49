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

bool aabb(double x1, double y1, double w1, double h1,
		double x2, double y2, double w2, double h2) {
	return !(x1 > (x2 + w2)
		|| (x1 + w1) < x2
		|| (y1 + h1) < y2
		|| y1 > (y2 + h2));
}

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
			p.yvel += 50;

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
						for (int i = 0; i < 4; i++)
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
			return state_ != state::falling && aabb(ox, oy, ow, oh, x, y, 8, 8);
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

	template <typename F>
	void add_platform(F &&check) {
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

			if (!ok || !check(xx, yy, len)) continue;

			add_platform_at(xx, yy, len);

			break;
		}
	}

	void add_platform_at(int x, int y, int len) {
		for (int i = 0; i < len; i++) {
			block b{prog_, part_, f_dist_(global_mt)};
			b.x = (x + i) * 8;
			b.y = y * 8;
			blocks_.emplace(glm::ivec2{x + i, y}, std::move(b));
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

	bool block_at(int x, int y) {
		return blocks_.contains(glm::ivec2{x, y});
	}

	bool solid_at(int x, int y) {
		return blocks_.contains(glm::ivec2{x, y})
			&& blocks_.at({x, y}).state_ != block::state::falling;
	}

private:
	gl::program &prog_;
	particles &part_;
	std::unordered_map<glm::ivec2, block> blocks_;
	std::uniform_int_distribution<int> l_dist_{2, 8};
	std::uniform_int_distribution<int> f_dist_{0, 23};
};

struct movement {
	bool left;
	bool right;
	bool jump;
};

struct entity {
	entity(blocks &blocks, gl::program &prog, int base_frame, double xspeed)
	: xspeed_{xspeed}, base_frame_{base_frame}, blocks_{blocks},
		spr_{prog, "res/player.png", 8, 8, base_frame} { }

	virtual ~entity() = default;

	entity(const entity &) = delete;
	entity(entity &&) = default;

	virtual movement get_current_movement(double delta, input_state &input) = 0;

	void tick(double delta, input_state &input) {
		auto mov = get_current_movement(delta, input);
		if (mov.left) {
			spr_.set_frame(spr_.get_frame() | 1);
			xdir = -1;
			xvel = xspeed_;
		}

		if (mov.right) {
			spr_.set_frame(spr_.get_frame() & ~1);
			xdir = 1;
			xvel = xspeed_;
		}

		if (mov.jump && jump_ctr) {
			if (xdir == -1)
				spr_.set_frame(base_frame_ + 9);
			if (xdir == 1)
				spr_.set_frame(base_frame_ + 8);
			yvel = -240;
			jump_ctr--;
			jump_frame_wait = 10;
		}

		constexpr double steps = 50;
		for (int i = 0; i < steps; i++) {
			auto newx = x + (xvel * xdir * delta) / steps;
			auto newy = y + (yvel * delta) / steps;

			if (!blocks_.check_collision(x, newy, 7, 7)) {
				y = newy;
				if ((yvel > 0 || yvel < 0) && !jump_frame_wait) {
					if (xdir == -1)
						spr_.set_frame(base_frame_ + 17);
					if (xdir == 1)
						spr_.set_frame(base_frame_ + 16);
				}
			} else {
				if (yvel > 0) {
					jump_ctr = 2;
					if (xdir == -1)
						spr_.set_frame(base_frame_ + 1);
					if (xdir == 1)
						spr_.set_frame(base_frame_ + 0);
					jump_frame_wait = 0;
				}
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
		}

		yvel += 10;
	}

	void render() {
		spr_.x = x;
		spr_.y = y;
		spr_.render();

		if (jump_frame_wait) jump_frame_wait--;
	}

	double get_x() const { return x; }
	double get_y() const { return y; }

	void set_position(double x, double y) {
		this->x = x; this->y = y;
	}

private:
	double xspeed_;
	int base_frame_;
	blocks &blocks_;
	sprite spr_;
	double x = 0, y = 0;
	double xvel = 0, yvel = 0;
	int xdir = 0;
	int jump_ctr = 2;
	int jump_frame_wait = 0;
};

struct player : entity {
	player(blocks &blocks, gl::program &prog)
	: entity{blocks, prog, 0, 130} { }

	virtual ~player() = default;

	movement get_current_movement(double, input_state &input) override {
		return {
			input.down_keys[SDLK_LEFT],
			input.down_keys[SDLK_RIGHT],
			input.just_pressed_keys.contains(SDLK_UP)
		};
	}
};

struct enemy : entity {
	enemy(player &player, blocks &blocks, gl::program &prog)
	: entity{blocks, prog, 2, 80}, player_{player}, blocks_{blocks} { }

	enemy(const enemy &) = delete;
	enemy(enemy &&) = default;

	virtual ~enemy() = default;

	movement get_current_movement(double, input_state &) override {
		if (dir == 1 && blocks_.check_collision(get_x() + 4, get_y(), 7, 7))
			dir = -dir;

		if (dir == 1 && !blocks_.check_collision(get_x() + 4, get_y() + 4, 7, 7))
			dir = -dir;

		if (dir == -1 && blocks_.check_collision(get_x() - 4, get_y(), 7, 7))
			dir = -dir;

		if (dir == -1 && !blocks_.check_collision(get_x() - 4, get_y() + 4, 7, 7))
			dir = -dir;

		bool left = dir == -1, right = dir == 1;
		if (!blocks_.check_collision(get_x(), get_y() + 4, 7, 7))
			left = right = false;

		std::uniform_int_distribution<int> dist{0, 34};

		wants_shoot_ = dist(global_mt) == 2;

		return {left, right, false};
	}

	bool wants_shoot() {
		return std::exchange(wants_shoot_, 0);
	}

	int facing() const {
		return dir;
	}

private:
	player &player_;
	blocks &blocks_;
	int dir = 1;
	bool wants_shoot_ = false;
};

struct bullets {
	bullets(blocks &blocks, gl::program &prog)
	: blocks_{blocks}, spr_{prog, "res/bullet.png", 2, 2} { }

	void tick(double delta) {
		for (auto it = pos_.begin(); it != pos_.end();) {
			auto &p = *it;
			p.x += delta * p.z;

			if (p.x >= window::width || p.x <= -2
					|| blocks_.check_collision(p.x, p.y, 1, 1))
				it = pos_.erase(it);
			else
				++it;
		}
	}

	void add_bullet(double x, double y, double xspeed) {
		pos_.push_back({x, y, xspeed});
	}

	void render() {
		for (auto &p : pos_) {
			spr_.x = p.x;
			spr_.y = p.y;
			spr_.render();
		}
	}

private:
	std::vector<glm::vec3> pos_;
	blocks &blocks_;
	sprite spr_;
};

std::string format_time(double time) {
	int cs = (int)(time * 10) % 10;
	int s = (int)(time) % 60;
	int m = (int)(time / 60);

	std::string out;

	if (m) {
		out += std::to_string(m) + ":";
	}

	if (m && s < 10) {
		out += "0";
	}

	out += std::to_string(s) + ".";
	out += std::to_string(cs);

	return out;
}

struct scene {
	scene() {
		player_.set_position(window::width / 2 - 4, window::height / 4);
		blocks_.add_platform_at((window::width / 8 - 8) / 2, 10, 8);
	}

	void tick(double delta, input_state &input) {
		time_tracker_.tick(delta);
		clouds_.tick();

		std::uniform_int_distribution<int> g_dist{0, 80};
		std::uniform_int_distribution<int> e_dist{0, 3};

		if (g_dist(global_mt) == 2)
			blocks_.add_platform(
			[&] (int x, int y, int len) {
				if (aabb(player_.get_x(), player_.get_y(), 7, 7,
						x * 8, y * 8, len * 8, 8))
					return false;

				for (auto &e : enemies_)
					if (aabb(e->get_x(), e->get_y(), 7, 7,
							x * 8, y * 8, len * 8, 8))
						return false;

				if (e_dist(global_mt) == 0) {
					if (blocks_.check_collision(x * 8 + len * 4, (y - 1) * 8, 7, 7))
						return false;

					enemies_.emplace_back(std::make_unique<enemy>(player_, blocks_, prog_));
					enemies_.back()->set_position(x * 8 + len * 4, (y - 1) * 8);
				}
				return true;
			});

		blocks_.tick(delta);
		particles_.tick(delta);
		for (auto it = enemies_.begin(); it != enemies_.end();) {
			auto &e = **it;
			e.tick(delta, input);
			if (e.wants_shoot()) {
				bullets_.add_bullet(e.get_x() + (e.facing() == 1 ? 8 : -3), e.get_y() + 2, e.facing() * 30);
			}
			if (e.get_y() >= window::height)
				it = enemies_.erase(it);
			else
				++it;
		}
		player_.tick(delta, input);
		bullets_.tick(delta);
	}

	void render() {
		prog_.set_uniform("ortho", ortho);

		clouds_.render();

		bg_.render();

		blocks_.render();
		player_.render();
		for (auto &e : enemies_)
			e->render();
		bullets_.render();
		particles_.render();

		text t{prog_, fnt_};

		double elapsed = time_tracker_.now() - start_at_;
		std::string text = "Time: " + format_time(elapsed);

		t.x = (window::width - text.size() * 6) / 2;
		t.y = 6;
		t.set_text(text);
		t.render({0, 0, 0, 1});
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
	std::vector<std::unique_ptr<enemy>> enemies_;

	bullets bullets_{blocks_, prog_};

	sprite bg_{prog_, "res/bg.png", 160, 120};

	double start_at_ = 0;

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
