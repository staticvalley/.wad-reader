#include <print>
#include <WADReader.hpp>

int main(void) {
	const char* path = "C:/Users/jrk/Desktop/cs_dust.wad";
	WADReader wad(path);

	wad.printHeader();

	wad.processTextures();

	for (auto s : wad.textures)
		std::println("{}", s.first);
}