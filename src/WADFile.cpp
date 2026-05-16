#include <WADFile.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <print>
#include <cstring>
#include <unordered_map>
#include <vector>

WADFile::WADFile() {};

WADFile::~WADFile() {};

bool WADFile::load(const char* path) {

	// open .wad
	std::ifstream fileStream = std::ifstream(path, std::ios::binary);
	if (!fileStream.is_open())
		return false;

	// read header, directories, and textures
	try {
		processHeader(fileStream);
		processEntries(fileStream);
		processTextures(fileStream);
	}
	catch (const std::runtime_error& e) {
		std::println(std::cerr, "WADFile: failed to load {}: {}", path, e.what());
		return false;
	}

	return true;
}

void WADFile::processHeader(std::ifstream& fileStream) {

	WADHeader newHeader;

	// read and verify BSP version
	fileStream.read((char*)(&newHeader.magic), sizeof(newHeader.magic));
	if (!fileStream)
		throw std::runtime_error(std::format("error reading WAD magic number"));

	// only reading WAD3 files
	if (std::strncmp(newHeader.magic, WAD_MAGIC, WAD_MAGIC_SIZE) != 0)
		throw std::runtime_error(std::format("invalid WAD version: {}", newHeader.magic));

	// read in entry information
	fileStream.read((char*)(&newHeader.nEntries), sizeof(newHeader.nEntries));
	if (!fileStream)
		throw std::runtime_error(std::format("error reading entry count"));

	// read in directory offset 
	fileStream.read((char*)(&newHeader.nDirOffset), sizeof(newHeader.nDirOffset));
	if (!fileStream)
		throw std::runtime_error(std::format("error reading directory offset"));
	
	header = std::move(newHeader);
}

void WADFile::processEntries(std::ifstream& fileStream) {
	
	std::vector<WADEntry> wadEntries;

	// seek to directory offset
	fileStream.seekg(header.nDirOffset, std::ios::beg);

	// read in each entry and add to entry vector
	for (int i = 0; i < header.nEntries; i++) {
		
		WADEntry newEntry;

		fileStream.read((char*)&newEntry, sizeof(WADEntry));
		if (!fileStream)
			throw std::runtime_error(std::format("error reading WAD directory entry {}", i + 1));

		wadEntries.push_back(newEntry);
	}

	entries = std::move(wadEntries);
}

void WADFile::processTextures(std::ifstream& fileStream) {
	
	// process texture for each entry
	for (int i = 0; i < entries.size(); i++) {
		
		WADMipTexture mipTexture;
		WADTexture texture;
		
		// only parse texture entries
		if (entries[i].nType != WAD_ENTRY_MIPTEX) continue;
	
		fileStream.seekg(entries[i].nFilePos, std::ios::beg);

		fileStream.read((char*)&mipTexture, sizeof(mipTexture));
		if (!fileStream)
			throw std::runtime_error(std::format("error reading WAD mip texture {}", i + 1));

		uint32_t textureSize = mipTexture.nWidth * mipTexture.nHeight;
		
		// seek to raw pixel data
		fileStream.seekg(entries[i].nFilePos + mipTexture.nOffsets[0], std::ios::beg);

		// read in raw pixel data
		std::vector<uint8_t> pixels(textureSize);
		fileStream.read((char*)pixels.data(), sizeof(uint8_t) * textureSize);
		if (!fileStream)
			throw std::runtime_error(std::format("error reading WAD mip \"{}\" texture pixel data", mipTexture.szName));
		texture.pixels = std::move(pixels);

		// palette offset is past 3rd mipmap texture
		uint32_t paletteOffset = entries[i].nFilePos + mipTexture.nOffsets[3] + ((mipTexture.nWidth / 8) * (mipTexture.nHeight / 8));
		// skip palette count
		fileStream.seekg(paletteOffset + sizeof(uint16_t), std::ios::beg);

		// read palette
		fileStream.read((char*)&texture.palette, sizeof(uint8_t) * RGB_PALETTE_SIZE);
		if (!fileStream)
			throw std::runtime_error(std::format("error reading WAD mip \"{}\" texture palette data", mipTexture.szName));

		texture.height = mipTexture.nHeight;
		texture.width = mipTexture.nWidth;
		std::strncpy(texture.name, mipTexture.szName, WAD_MAX_TEXTURE_NAME);

		_textures[mipTexture.szName] = texture;
	}
}