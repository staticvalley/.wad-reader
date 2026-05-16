#pragma once

#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/*
	using this as my guide:
	https://developer.valvesoftware.com/wiki/WAD
*/

#pragma once

#include <vector>
#include <array>

enum WADEntryType : int8_t {
    WAD_ENTRY_PALETTE = 0x40,
    WAD_ENTRY_COLORMAP = 0x41,
    WAD_ENTRY_QPIC = 0x42,
    WAD_ENTRY_MIPTEX = 0x43, // impl
    WAD_ENTRY_RAW = 0x44,
    WAD_ENTRY_COLORMAP2 = 0x45,
    WAD_ENTRY_FONT = 0x46,
};

#define WAD_MAGIC "WAD3"
#define WAD_MAGIC_SIZE 4
struct WADHeader {
    char magic[WAD_MAGIC_SIZE]; // WAD_MAGIC for goldsrc
    int32_t nEntries;           // number of entries in directory
    int32_t nDirOffset;         // offset to directory entries
};

#define WAD_MAX_TEXTURE_NAME 16

struct WADEntry {
    int32_t nFilePos;                   // offset in WAD
    int32_t nDiskSize;                  // size in file
    int32_t nSize;                      // uncompressed size
    WADEntryType nType;                 // type of entry
    bool bCompression;                  // 0 if none
    int16_t nDummy;                     // not used
    char szName[WAD_MAX_TEXTURE_NAME];  // must be null terminated
};

#define WAD_MIP_LEVELS 4

struct WADMipTexture {
    char szName[WAD_MAX_TEXTURE_NAME];  // name of texture
    uint32_t nWidth, nHeight;           // extends of the texture
    uint32_t nOffsets[WAD_MIP_LEVELS];  // offsets to texture mipmaps BSPMIPTEX;
};

#define RGB_PALETTE_SIZE 256 * 3
struct WADTexture {
    char name[WAD_MAX_TEXTURE_NAME];	                // name of texture
    uint32_t width = 0, height = 0;		                // dimensions
    std::vector<uint8_t> pixels;		                // raw texture pixel data
    std::array<uint8_t, RGB_PALETTE_SIZE> palette{};    // palette data
};

class WADFile {
public:

    WADFile();
    ~WADFile();

    // only moves, no copies
    WADFile(WADFile&&) = default;
    WADFile& operator=(WADFile&&) = default;
    WADFile(const WADFile&) = delete;
    WADFile& operator=(const WADFile&) = delete;

    // load WAD3 file from path
    bool load(const char* path);

    // getters
    const std::unordered_map<std::string, WADTexture>& textures() const { return _textures; }

private:
    WADHeader header{};
	std::vector<WADEntry> entries;

    // texture name to WADTexture map
    std::unordered_map<std::string, WADTexture> _textures;
	
	// parse WAD version and directory data into header struct
	void processHeader(std::ifstream& fileStream);

	// parse entries from directory into vector
	void processEntries(std::ifstream& fileStream);
	
    // process texture sections of wad into <name, texture> hashmap
    void processTextures(std::ifstream& fileStream);
};
