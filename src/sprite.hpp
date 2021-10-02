#pragma once

#include <array>
#include <gl/texture.hpp>
#include <gl/mesh.hpp>
#include <gl/shader.hpp>

struct sprite {
	sprite(gl::program &prog, const std::string &texture, int w, int h, int f = 0)
	: mesh_{&prog}, w_{w}, h_{h}, frame_{f} {
		tex_.load(texture);

		vtx_ = std::array<gl::vertex, 6>{
			gl::vertex{{0, 0}, {0, 0}, {1, 1, 1, 1}},
			gl::vertex{{w, 0}, {1, 0}, {1, 1, 1, 1}},
			gl::vertex{{w, h}, {1, 1}, {1, 1, 1, 1}},

			gl::vertex{{0, 0}, {0, 0}, {1, 1, 1, 1}},
			gl::vertex{{w, h}, {1, 1}, {1, 1, 1, 1}},
			gl::vertex{{0, h}, {0, 1}, {1, 1, 1, 1}}
		};

		set_frame(frame_);
	}

	void render() {
		tex_.bind();
		mesh_.program()->set_uniform("obj_pos", glm::vec2{x, y});
		mesh_.render();
	}

	void set_frame(int frame) {
		frame_ = frame;

		int per_x = tex_.width() / w_;

		float tx = ((frame % per_x) * w_) / static_cast<float>(tex_.width());
		float ty = ((frame / per_x) * h_) / static_cast<float>(tex_.height());

		float tw = tx + w_ / static_cast<float>(tex_.width());
		float th = ty + h_ / static_cast<float>(tex_.height());

		vtx_[0].tex = {tx, ty};
		vtx_[1].tex = {tw, ty};
		vtx_[2].tex = {tw, th};
		vtx_[3].tex = {tx, ty};
		vtx_[4].tex = {tw, th};
		vtx_[5].tex = {tx, th};

		mesh_.vbo().store_regenerate(vtx_.data(), vtx_.size() * sizeof(gl::vertex), GL_STATIC_DRAW);
	}

	int get_frame() const {
		return frame_;
	}

	int x = 0, y = 0;

private:
	std::array<gl::vertex, 6> vtx_{};
	gl::texture2d tex_;
	gl::mesh mesh_;

	int w_, h_;
	int frame_;
};
