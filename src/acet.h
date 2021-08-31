#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "lodepng.h"

const int kSaveSize = 0x4440;
const int kImageOffset = 0x24; // The actual image data starts at offset 0x24
const int kImageWidth = 128;
const int kImageHeight = 128;
const int kNumPixels = 0x4000; // 128*128 pixels
const int kPaletteSize = 0x400; // 256 colors * 4 channels

const uint8_t kEmblemHeader[12] = {
    0x20, 0x44, 0x00, 0x00, 0x20, 0x44, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00
};

size_t bufferIndex(size_t i, size_t emblem_offset);

uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

uint8_t UnscramblePalette(uint8_t index);

bool isPNG(std::string filename);

size_t ReadSave(std::string filename, std::vector<uint8_t> &buffer);

void InjectImage(std::string savename, std::string imagename);

void WriteImage(std::string savename, std::string filename = "output");
