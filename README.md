# Ancient Pixels

Ancient Pixels is my entry for Ludum Dare 49 ([https://ldjam.com/events/ludum-dare/49/ancient-pixels](link)).

## Compiling

You will need:
 - Emscripten (new enough to support C++20)
 - Meson and Ninja

Clone the repository with submodules:
```
$ git clone https://github.com/qookei/ld49 --recursive
```

Then configure it:
```
$ meson build --cross-file=cross/emscripten.txt
```

Then compile it:
```
$ ninja -C build
```

Then, create a symlink for `ld49.data` and launch a web server to test it, for example:
```
$ ln -s build/ld49.data ld49.data
$ python -m http.server
```
