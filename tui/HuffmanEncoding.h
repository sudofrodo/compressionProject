#ifndef HUFFMAN_ENCODING_H
#define HUFFMAN_ENCODING_H

#include <string>
#include <unordered_map>
#include <fstream>

namespace Huffman {

    struct Node {
        char ch;
        int freq;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    extern std::string stats;

    void heapify_min_heap(Node* array[], int size, int root);
    void build_min_heap(Node* array[], int size);
    Node* delete_min(Node* heap[], int &size);

    std::unordered_map<char, int> frequencyTable(std::string text);
    Node* huffmanTree(Node* heap[], int &size);
    void generateCodes(Node* root, std::string currentCode, std::unordered_map<char, std::string>& codes);

    std::string encode(const std::string& text, std::unordered_map<char, std::string>& codes);
    std::string decode(const std::string& encoded, Node* root);
    std::string readTextFile(const std::string& filename);

    void serializeTree(Node* root, std::ofstream& file);
    Node* deserializeTree(std::ifstream& file);

    void packBits(const std::string& encoded, std::ofstream& file);
    std::string unpackBits(std::ifstream& file);

    std::string getNewFilename(std::string filename, char type);
    std::string showStats(int originalSize, int compressedSize, double durationMs);

    void huffmanFileCompress(const std::string& filename);
    std::string readCompressedFile(const std::string& filename, Node*& root);
    void huffmanFileDecompress(const std::string& filename);

    double GetCompressionRatio();
    int GetCompressionTime();

} // namespace Huffman

#endif // HUFFMAN_ENCODING_H