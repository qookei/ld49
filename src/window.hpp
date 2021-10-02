#pragma once

#include <emscripten.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <map>
#include <set>
#include <cmath>
#include <iostream>

// #define LOG_SCALE

struct input_state {
	int mouse_x, mouse_y;
	std::map<SDL_Keycode, bool> down_keys;
	std::set<SDL_Keycode> just_pressed_keys;
};

struct window {
	static constexpr int width = 160;
	static constexpr int height = 120;

	window() {
		SDL_Init(SDL_INIT_VIDEO);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetSwapInterval(1);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		int client_width, client_height;
		client_width = EM_ASM_INT({return document.documentElement.clientWidth}, 0);
		client_height = EM_ASM_INT({return document.documentElement.clientHeight}, 0);

		scale_ = std::min(client_width / width, client_height / height);

		if (scale_ < 1) {
			std::cerr << "Browser window too small to fit canvas!\n";
			scale_ = 1;
		}

#ifdef LOG_SCALE
		std::cout << "Computed scale is " << scale_
			<< " (viewport: " << width << "x" << height << ")"
			<< " (client: " << client_width << "x" << client_height << ")\n";
#endif

		wnd_ = SDL_CreateWindow("LD49", SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				width * scale_, height * scale_,
				SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

		ctx_ = SDL_GL_CreateContext(wnd_);
		glViewport(0, 0, width * scale_, height * scale_);

		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false,
		[] (int, const EmscriptenUiEvent *ui_ev, void *ctx) -> int {
			auto wnd = static_cast<window *>(ctx);

			int client_width = ui_ev->documentBodyClientWidth;
			int client_height = ui_ev->documentBodyClientHeight;
			wnd->scale_ = std::min(client_width / width, client_height / height);

			if (wnd->scale_ < 1) {
				std::cerr << "Browser window too small to fit canvas!\n";
				wnd->scale_ = 1;
			}

#ifdef LOG_SCALE
			std::cout << "Computed scale is " << wnd->scale_
				<< " (viewport: " << width << "x" << height << ")"
				<< " (client: " << client_width << "x" << client_height << ")\n";
#endif

			SDL_SetWindowSize(wnd->wnd_, width * wnd->scale_, height * wnd->scale_);
			glViewport(0, 0, width * wnd->scale_, height * wnd->scale_);

			return true;
		});
	}

	template <typename T>
	void attach_ticker(T &ticker) {
		ticker_ctx_ = &ticker;
		ticker_cb_ = [] (double delta, const input_state &input, void *ctx) {
			static_cast<T *>(ctx)->tick(delta, input);
		};
	}

	template <typename T>
	void attach_renderer(T &renderer) {
		renderer_ctx_ = &renderer;
		renderer_cb_ = [] (void *ctx) {
			static_cast<T *>(ctx)->render();
		};
	}

	void enter_main_loop() {
		last_ticks_ = SDL_GetTicks();
		emscripten_set_main_loop_arg([] (void *ctx) {
			static_cast<window *>(ctx)->main_loop();
		}, this, 0, true);
	}

	void main_loop() {
		auto now_ticks = SDL_GetTicks();
		auto delta = static_cast<double>(now_ticks - last_ticks_) / 1000.0;
		last_ticks_ = now_ticks;

		input_.just_pressed_keys.clear();

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_KEYUP:
					input_.down_keys[ev.key.keysym.sym] = false;
					break;
				case SDL_KEYDOWN:
					if (!input_.down_keys[ev.key.keysym.sym])
						input_.just_pressed_keys.insert(ev.key.keysym.sym);
					input_.down_keys[ev.key.keysym.sym] = true;
					break;
			}
		}

		ticker_cb_(delta, input_, ticker_ctx_);

		glClearColor(0.364f, 0.737f, 0.823f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer_cb_(renderer_ctx_);

		SDL_GL_SwapWindow(wnd_);
	}

	~window() {
		SDL_GL_DeleteContext(ctx_);
		SDL_DestroyWindow(wnd_);
		SDL_Quit();
	}

	window(const window &) = delete;
	window(window &&) = delete;

	window &operator=(const window &) = delete;
	window &operator=(window &&) = delete;

private:
	SDL_Window *wnd_ = nullptr;
	SDL_GLContext ctx_;
	int scale_ = 1;

	uint32_t last_ticks_ = 0;
	input_state input_{};

	void *ticker_ctx_ = nullptr;
	void (*ticker_cb_)(double, const input_state &, void *) = nullptr;

	void *renderer_ctx_ = nullptr;
	void (*renderer_cb_)(void *) = nullptr;
};
