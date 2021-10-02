#pragma once

#include <gl/vertex.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>

namespace gl {

struct mesh {
	friend void swap(mesh &a, mesh &b) {
		using std::swap;
		swap(a.prog_, b.prog_);
		swap(a.vbo_, b.vbo_);
	}

	mesh(program *prog = nullptr)
	: vbo_{}, prog_{prog} {
		vbo_.generate();
	}

	mesh(const mesh &other) = delete;
	mesh(mesh &&other)
	: mesh{} {
		swap(*this, other);
	}

	mesh &operator=(const mesh &other) = delete;
	mesh &operator=(mesh &&other) {
		swap(*this, other);
		return *this;
	}

	template <GLenum Mode = GL_TRIANGLES>
	void render() const {
		vbo_.bind();
		prog_->use();
		glDrawArrays(Mode, 0, vbo_.size() / sizeof(gl::vertex));
	}

	template <GLenum Mode = GL_TRIANGLES>
	void render(size_t n_vertices) const {
		vbo_.bind();
		prog_->use();
		glDrawArrays(Mode, 0, n_vertices);
	}

	vertex_buffer &vbo() {
		return vbo_;
	}

	program *program() const {
		return prog_;
	}

private:
	vertex_buffer vbo_;
	struct program *prog_;
};

} // namespace gl
