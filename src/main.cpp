#include <print>
#include <WADFile.hpp>

int main(void) {
	const char* path = "C:/Users/jrk/Desktop/cs_dust.wad";
	WADFile wad;

	wad.load(path);

	for (auto s : wad.textures())
		std::println("{}", s.first);
}