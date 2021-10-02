#pragma once

#include <string_view>
#include <iostream>
#include <fstream>
#include <gl/mesh.hpp>
#include <gl/texture.hpp>

struct text;

struct font {
	friend struct text;

	font(const std::string &resource) {
		std::ifstream res{resource};
		if (!res) {
			std::cerr << "Cannot open font resource \"" << resource << "\"\n";
			return;
		}

		std::string path;
		std::getline(res, path);

		res >> char_w_ >> char_h_ >> chars_per_atlas_line_;

		atlas_.load(path);

		std::cout << "Loaded font \"" << path << "\" with metrics " << char_w_ << " " << char_h_ << " " << chars_per_atlas_line_ << "\n";
	}

private:
	gl::texture2d atlas_;
	int char_w_;
	int char_h_;
	int chars_per_atlas_line_;
};

struct text {
	text(gl::program &prog, font &font)
	: font_{&font}, mesh_{&prog} {}

	void set_text(std::string_view str) {
		n_chars_ = 0;

		for (char c : str)
			if (c && c != '\n' && c != ' ')
				n_chars_++;

		size_t new_size = n_chars_ * 6;

		std::vector<gl::vertex> verts;
		verts.resize(new_size);

		int x = 0, y = 0;
		size_t i = 0;

		for (char c : str) {
			if (!c || c == ' ') {
				x += font_->char_w_;
				continue;
			}

			if (c == '\n') {
				x = 0;
				y += font_->char_h_;
				continue;
			}

			float tx = ((c % font_->chars_per_atlas_line_) * font_->char_w_) / static_cast<float>(font_->atlas_.width());
			float ty = ((c / font_->chars_per_atlas_line_) * font_->char_h_) / static_cast<float>(font_->atlas_.height());

			float tw = tx + font_->char_w_ / static_cast<float>(font_->atlas_.width());
			float th = ty + font_->char_h_ / static_cast<float>(font_->atlas_.height());

			int w = x + font_->char_w_, h = y + font_->char_h_;

			verts[i++] = {{x, y}, {tx, ty}, {1, 1, 1, 1}};
			verts[i++] = {{w, y}, {tw, ty}, {1, 1, 1, 1}};
			verts[i++] = {{w, h}, {tw, th}, {1, 1, 1, 1}};

			verts[i++] = {{x, y}, {tx, ty}, {1, 1, 1, 1}};
			verts[i++] = {{w, h}, {tw, th}, {1, 1, 1, 1}};
			verts[i++] = {{x, h}, {tx, th}, {1, 1, 1, 1}};

			x += font_->char_w_;
		}

		if (mesh_.vbo().size() < new_size)
			mesh_.vbo().store_regenerate(verts.data(), new_size * sizeof(gl::vertex), GL_STATIC_DRAW);
		else
			mesh_.vbo().store(verts.data(), 0, new_size * sizeof(gl::vertex));
	}

	void render() {
		font_->atlas_.bind();
		mesh_.program()->set_uniform("obj_pos", glm::vec2{x, y});
		mesh_.render(n_chars_ * 6);
	}

	int x = 0, y = 0;

private:
	font *font_;
	gl::mesh mesh_;
	size_t n_chars_;
};
