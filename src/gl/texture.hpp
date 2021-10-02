#pragma once

#include <SDL2/SDL_image.h>
#include <GLES2/gl2.h>
#include <cassert>
#include <iostream>

namespace gl {

struct texture2d {
	friend void swap(texture2d &a, texture2d &b) {
		using std::swap;
		swap(a.id_, b.id_);
		swap(a.surf_, b.surf_);
	}

	texture2d()
	: id_{}, surf_{} { }

	~texture2d() {
		SDL_FreeSurface(surf_);
		glDeleteTextures(1, &id_);
	}

	texture2d(const texture2d &) = delete;
	texture2d &operator=(const texture2d &) = delete;

	texture2d(texture2d &&other)
	: texture2d() {
		swap(*this, other);
	}

	texture2d &operator=(texture2d &&other) {
		swap(*this, other);
		return *this;
	}

	void generate() {
		glGenTextures(1, &id_);
		glBindTexture(GL_TEXTURE_2D, id_);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void bind() const {
		glBindTexture(GL_TEXTURE_2D, id_);
	}

	void load(const std::string &path) {
		surf_ = IMG_Load(path.data());

		if (surf_) {
			restore();
		} else {
			std::cerr << __func__ << ": failed to load \"" << path << "\"" << std::endl;
			assert(!"failed to load texture");
		}
	}

	void restore() {
		if (surf_) {
			auto mode = GL_RGB;
			if (surf_->format->BytesPerPixel == 4)
				mode = GL_RGBA;

			generate();

			glTexImage2D(GL_TEXTURE_2D, 0, mode, surf_->w, surf_->h, 0, mode, GL_UNSIGNED_BYTE, surf_->pixels);
		}
	}

	int width() const {
		return surf_->w;
	}

	int height() const {
		return surf_->h;
	}

private:
	GLuint id_;

	SDL_Surface *surf_ = nullptr;
};

} // namespace gl
