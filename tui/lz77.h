#ifndef LZ77_H
#define LZ77_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include "LinkedList.h"

extern const int MAX_WINDOW_SIZE;

LinkedList lz77Compress(std::string text);
std::string lz77Decompress(LinkedList ledger);
std::string tripletsToString(LinkedList ledger);
LinkedList textfileToTriplets(std::ifstream &inFile);
LinkedList binaryFileToTriplets(std::ifstream &inFile);
std::string getNewFilename(std::string filename, char type);
std::string getCompressionStats(std::string filename);
void compressToBinaryFile(std::string filename);
void decompressToTextFile(std::string filename);
double GetCompressionRatio();
int GetCompressionTime();

#endif