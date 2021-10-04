project('ld49', 'cpp',
	default_options : [
		'warning_level=3',
		'buildtype=release',
		'cpp_rtti=false',
		'cpp_exceptions=false',
		'cpp_std=c++20'
	])

sources = files(
	'src/main.cpp'
)

resources = files(
	'res/shaders/generic-vertex.glsl',
	'res/shaders/generic-fragment.glsl',
	'res/font.png',
	'res/font.txt',
	'res/cloud.png',
	'res/blocks.png',
	'res/bg.png',
	'res/particle.png',
	'res/player.png',
	'res/healthbar.png',
	'res/powerbar.png',

	'res/sound/block-fall.wav',
	'res/sound/gameover.wav',
	'res/sound/hit.wav',
	'res/sound/jump.wav',
	'res/sound/pickup.wav',
	'res/sound/shoot.wav',
)

deps = []

cmake = import('cmake')

glm_opts = cmake.subproject_options()
glm_opts.add_cmake_defines({'GLM_TEST_ENABLE' : false})
glm = cmake.subproject('glm', options : glm_opts)
deps += glm.dependency('glm')

if host_machine.system() == 'emscripten'
	deps += declare_dependency(
		compile_args : ['-s', 'USE_SDL=2'],
		link_args : ['-s', 'USE_SDL=2']
	)
	deps += declare_dependency(
		compile_args : ['-s', 'USE_SDL_IMAGE=2'],
		link_args : ['-s', 'USE_SDL_IMAGE=2']
	)
	deps += declare_dependency(
		compile_args : ['-s', 'USE_SDL_MIXER=2'],
		link_args : ['-s', 'USE_SDL_MIXER=2']
	)
else
	deps += dependency('SDL2')
	deps += dependency('SDL2_image')
	deps += dependency('SDL2_mixer')
endif

exe = executable('ld49',
	sources,
	include_directories : 'src/',
	dependencies : deps,
	link_args : ['--preload-file', meson.project_source_root() / 'res@/res', '--use-preload-plugins'],
	link_depends : resources
)
