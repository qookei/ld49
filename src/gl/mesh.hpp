#pragma once

#include <gl/vertex.hpp>
#include <gl/buffer.hpp>

namespace gl {

struct mesh {
	friend void swap(mesh &a, mesh &b) {
		using std::swap;
		swap(a.vbo_, b.vbo_);
	}

	mesh()
	: vbo_{} {
		vbo_.generate();
	}

	mesh(const mesh &other) = delete;
	mesh(mesh &&other) {
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
		glDrawArrays(Mode, 0, vbo_.size() / sizeof(gl::vertex));
	}

	template <GLenum Mode = GL_TRIANGLES>
	void render(size_t n_vertices) const {
		vbo_.bind();
		glDrawArrays(Mode, 0, n_vertices);
	}

	vertex_buffer &vbo() {
		return vbo_;
	}

private:
	vertex_buffer vbo_;
};

} // namespace gl
