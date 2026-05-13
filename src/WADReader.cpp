#include <WADReader.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <print>
#include <cstring>
#include <unordered_map>
#include <vector>

WADReader::WADReader(const char* path) 
	: filePath(path)
{

	fileStream = std::ifstream(path, std::ios::binary);
	if (!fileStream.is_open())
		throw std::runtime_error(std::format("failed to open BSP file: {}", filePath));

	// read in header from file
	header = processHeader();

	// only reading WAD3 files
	if (std::strncmp(header.magic, WAD_MAGIC, WAD_MAGIC_SIZE) != 0)
		throw std::runtime_error(std::format("invalid BSP version: {}", header.magic));

	entries = processEntries();
}

WADReader::~WADReader() {}

void WADReader::printHeader() {

	std::println("filepath: \"{}\"", filePath);
	std::println("entries: {}", header.nEntries);
	std::println("directory offset: {}", header.nDirOffset);

}

WADHeader WADReader::processHeader() {

	WADHeader newHeader;

	// read and verify BSP version
	fileStream.read((char*)(&newHeader.magic), sizeof(newHeader.magic));
	if (!fileStream) {
		std::cerr << "error reading WAD magic number" << std::endl;
		return {};
	}

	// read in entry information
	fileStream.read((char*)(&newHeader.nEntries), sizeof(newHeader.nEntries));
	if (!fileStream) {
		std::cerr << "error reading entry count" << std::endl;
		return {};
	}

	// read in directory offset 
	fileStream.read((char*)(&newHeader.nDirOffset), sizeof(newHeader.nDirOffset));
	if (!fileStream) {
		std::cerr << "error reading directory offset" << std::endl;
		return {};
	}
	
	return newHeader;
}

std::vector<WADEntry> WADReader::processEntries() {
	
	std::vector<WADEntry> wadEntries;

	// seek to directory offset
	fileStream.seekg(header.nDirOffset, std::ios::beg);

	// read in each entry and add to entry vector
	for (int i = 0; i < header.nEntries; i++) {
		
		WADEntry newEntry;

		fileStream.read((char*)&newEntry, sizeof(WADEntry));
		if (!fileStream)
			throw std::runtime_error("error reading WAD directory entries");

		wadEntries.push_back(newEntry);
	}

	return wadEntries;
}

void WADReader::processTextures() {
	
	// process texture for each entry
	for (int i = 0; i < entries.size(); i++) {
		
		WADMipTexture mipTexture;
		WADTexture texture;
		
		// only parse texture entries
		if (entries[i].nType != WAD_ENTRY_MIPTEX) continue;
	
		fileStream.seekg(entries[i].nFilePos, std::ios::beg);

		fileStream.read((char*)&mipTexture, sizeof(mipTexture));
		if (!fileStream)
			throw std::runtime_error("error reading WAD mip texture");

		uint32_t textureSize = mipTexture.nWidth * mipTexture.nHeight;
		
		// seek to raw pixel data
		fileStream.seekg(entries[i].nFilePos + mipTexture.nOffsets[0], std::ios::beg);

		// read in raw pixel data
		std::vector<uint8_t> pixels(textureSize);
		fileStream.read((char*)pixels.data(), sizeof(uint8_t) * textureSize);
		if (!fileStream)
			throw std::runtime_error("error reading WAD mip texture");
		texture.pixels = std::move(pixels);

		// palette offset is past 3rd mipmap texture
		uint32_t paletteOffset = entries[i].nFilePos + mipTexture.nOffsets[3] + ((mipTexture.nWidth / 8) * (mipTexture.nHeight / 8));
		// skip palette count
		fileStream.seekg(paletteOffset + sizeof(uint16_t), std::ios::beg);

		// read palette
		fileStream.read((char*)&texture.palette, sizeof(uint8_t) * RGB_PALETTE_SIZE);
		if (!fileStream)
			throw std::runtime_error("error reading WAD mip texture");

		texture.height = mipTexture.nHeight;
		texture.width = mipTexture.nWidth;
		std::strncpy(texture.name, mipTexture.szName, WAD_MAX_TEXTURE_NAME);

		textures[mipTexture.szName] = texture;
	}
}