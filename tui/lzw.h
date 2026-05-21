#ifndef LZW_H
#define LZW_H

#include <string>

namespace LZW {

    class Node {
    public:
        std::string data;
        Node* next;

        Node(std::string value);
    };

    class LinkedList {
    private:
        Node* head;

    public:
        LinkedList();
        void push_back(std::string value);
        std::string get(int index);
        int size();
    };

    extern std::string stats;

    std::string getNewFilename(std::string filename, char type);
    std::string showStats(int originalSize, int compressedSize, double durationMs);
    void compress(const std::string& inputFile);
    void decompress(const std::string& inputFile);
    double GetCompressionRatio();
    int GetCompressionTime();

} // namespace LZW

#endif // LZW_H