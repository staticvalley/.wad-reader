# WAD3 reader

a simple reader of the .wad (wad3) format of goldsrc map files. Created as a hobby parser for using valves .wad files and ones created and compiled in the J.A.C.K editor into my own 3D opengl engine.

this is a companion to the [.bsp reader](https://github.com/staticvalley/.bsp-reader) and [opengl fps](https://github.com/staticvalley/opengl-fps) projects I am working on.

#### Usage:

The reader can be used as so:

```
#include <WADReader.hpp>

int main(void) {
	const char* path = "assets/textures/de_dust2.wad";
	WADReader wad(path);

	wad.printHeader();
	
	wad.processTextures();

	WADTexture crate = wad.textures["MltryCrteSd"];
}
```

#### Todo/Known Issues:

- can only read mipmap texture entry types (as of now)
- invalid file handling needs work: missing a few edge cases and could more "gracefully" handle errors.
- only mipmap level 0 (unscaled) is parsed into the `WADTexture` type (missing 1/2, 1/4, 1/8 textures)
- could probably just be incorporated into the BSPReader