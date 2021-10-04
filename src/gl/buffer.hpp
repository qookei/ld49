#pragma once

#include <GLES2/gl2.h>
#include <cassert>
#include <utility>
#include <stddef.h>

namespace gl {

template <GLenum Type>
struct buffer {
	friend void swap(buffer &a, buffer &b) {
		using std::swap;

		swap(a.id_, b.id_);
		swap(a.size_, b.size_);
		swap(a.usage_, b.usage_);
	}

	buffer()
	:id_{0}, size_{0}, usage_{0} {}

	~buffer() {
		glDeleteBuffers(1, &id_);
	}

	buffer(const buffer &other) = delete;
	buffer(buffer &&other) {
		swap(*this, other);
	}

	buffer &operator=(const buffer &other) = delete;
	buffer &operator=(buffer &&other) {
		swap(*this, other);
		return *this;
	}

	void bind() const {
		assert(id_);
		glBindBuffer(Type, id_);
	}

	void unbind() const {
		glBindBuffer(Type, 0);
	}

	void generate() {
		glGenBuffers(1, &id_);
		bind();
	}

	void store(const void *data, size_t offset, size_t size) {
		if (!data)
			return;

		assert(size_ >= (size + offset) && "buffer overflow");

		bind();
		glBufferSubData(Type, offset, size, data);
	}

	void store_regenerate(const void *data, size_t size, GLenum usage) {
		if (size_ == size && usage_ == usage) {
			store(data, 0, size);
			return;
		}

		size_ = size;
		usage_ = usage;

		bind();
		glBufferData(Type, size_, data, usage);
	}

	void upload(const void *data, size_t size, GLenum usage) {
		assert(data);

		bind();
		if (size > size_) {
			usage_ = usage;
			size_ = size;
			glBufferData(Type, size_, data, usage_);
		} else {
			glBufferSubData(Type, 0, size_, data);
		}
	}

	size_t size() const {
		return size_;
	}

	GLenum usage() const {
		return usage_;
	}

	GLuint id() const {
		return id_;
	}

private:
	GLuint id_;
	size_t size_;
	GLenum usage_;
};

using vertex_buffer = buffer<GL_ARRAY_BUFFER>;

} // namespace gl
