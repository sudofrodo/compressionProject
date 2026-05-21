#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include "HuffmanEncoding.h"

using namespace std;

namespace Huffman {
    double compressionratio = 0.0;
    int compressiontime = 0;
    double GetCompressionRatio() {
    return compressionratio;
}

int GetCompressionTime(){
    return compressiontime;
}
    string stats = "";

    void heapify_min_heap(Node* array[], int size, int root) {
      int l = 2 * root + 1;
      int r = 2 * root + 2;
      int smallest = root;

      if (l < size && array[l]->freq < array[smallest]->freq) {
        smallest = l;
      }
      if (r < size && array[r]->freq < array[smallest]->freq) {
        smallest = r;
      }
      if (smallest != root) {
        Node* temp = array[root];
        array[root] = array[smallest];
        array[smallest] = temp;
        heapify_min_heap(array, size, smallest);
      }
    }

    void build_min_heap(Node* array[], int size){
      for (int i = size - 1; i >= 0; i--) {
        heapify_min_heap(array, size, i);
      }
    }

    Node* delete_min(Node* heap[], int &size){
      if (size <= 0) return nullptr;
      Node* min_freq_node = heap[0];
      heap[0] = heap[size-1];
      size--;
      heapify_min_heap(heap, size, 0);
      return min_freq_node;
    }

    unordered_map<char, int> frequencyTable(string text) {
        unordered_map<char, int> freqMap;
        for(char c : text){
            freqMap[c]++;
        }
        return freqMap;
    }

    Node* huffmanTree(Node* heap[], int &size) {
        while(size > 1){
            Node* left = delete_min(heap, size);
            Node* right = delete_min(heap, size);
        
            Node* internal_node = new Node;
            internal_node->freq = left->freq + right->freq;
            internal_node->ch = '#';
            internal_node->left = left;
            internal_node->right = right;

            heap[size] = internal_node;
            size++;
            build_min_heap(heap, size);
        }
        return heap[0];
    }

    void generateCodes(Node* root, string currentCode, unordered_map<char, string>& codes) {
        if(root->left == nullptr && root->right == nullptr){
            codes[root->ch] = currentCode;
            return;
        }
        if(root->right != nullptr){
            generateCodes(root->right, currentCode + "1", codes);
        }
        if(root->left != nullptr){
            generateCodes(root->left, currentCode + "0", codes);
        }
    }

    string encode(const string& text, unordered_map<char, string>& codes) {
        string result;
        for(char c : text){
            result += codes[c];
        }
        return result;
    }

    string decode(const string& encoded, Node* root) {
        string result;
        Node* current = root;
        for(char bit : encoded) {
            if(bit == '0'){
                current = current->left;
            }
            else if(bit == '1'){
                current = current->right;
            } 
            if(current->left == nullptr && current->right == nullptr){
                result += current->ch;
                current = root;
            }
        }
        return result;
    }

    string readTextFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: cannot open " << filename << endl;
            return "";
        }
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        return content;
    }

    void serializeTree(Node* root, ofstream& file) {
        if (!root) return;
        if (!root->left && !root->right) {
            char flag = '1';
            file.write(&flag, sizeof(char));
            file.write(&root->ch, sizeof(char));
        } else {
            char flag = '0';
            file.write(&flag, sizeof(char));
            serializeTree(root->left, file);
            serializeTree(root->right, file);
        }
    }

    Node* deserializeTree(ifstream& file) {
        char flag;
        if (!file.read(&flag, sizeof(char))) return nullptr;

        if (flag == '1') {
            Node* leaf = new Node();
            file.read(&leaf->ch, sizeof(char));
            return leaf;
        }
        Node* internal  = new Node();
        internal->ch    = '#';
        internal->left  = deserializeTree(file);
        internal->right = deserializeTree(file);
        return internal;
    }

    void packBits(const string& encoded, ofstream& file) {
        int totalBits = encoded.size();
        file.write((char*)&totalBits, sizeof(int));

        int i = 0;
        while (i < (int)encoded.size()) {
            unsigned char byte = 0;
            for (int pos = 7; pos >= 0 && i < (int)encoded.size(); pos--, i++) {
                if (encoded[i] == '1') {
                    byte = byte | (1 << pos);
                }
            }
            file.write((char*)&byte, sizeof(unsigned char));
        }
    }

    string unpackBits(ifstream& file) {
        int totalBits;
        file.read((char*)&totalBits, sizeof(int));

        string encoded = "";
        int bitsRead = 0;

        while (bitsRead < totalBits) {
            unsigned char byte;
            file.read((char*)&byte, sizeof(unsigned char));
            for (int pos = 7; pos >= 0 && bitsRead < totalBits; pos--, bitsRead++) {
                int bit = (byte >> pos) & 1;
                encoded += (bit == 1) ? '1' : '0';
            }
        }
        return encoded;
    }

    string getNewFilename(string filename, char type){
        size_t dash = filename.find_last_of("-");
        string baseName = filename;
        if(type == 't') {
            if (dash != string::npos) {
                baseName = filename.substr(0, dash) + ".txt";
            }
        } else {
            size_t lastDot = filename.find_last_of(".");
            if (lastDot != string::npos) {
                baseName = filename.substr(0, lastDot) + "-huff.bin";
            }
        }
        return baseName;
    }

    string showStats(int originalSize, int compressedSize, double durationMs) {

        double compressionPercentage = 0.0;
        if (originalSize > 0) {
            compressionPercentage = (1.0 - (static_cast<double>(compressedSize) / originalSize)) * 100.0;
        }

        string ratioStr = to_string(compressionPercentage);
        if (ratioStr.find('.') != string::npos) {
            while (ratioStr.back() == '0') ratioStr.pop_back();
            if (ratioStr.back() == '.') ratioStr.pop_back();
        }

        string timeStr = to_string(durationMs);
        if (timeStr.find('.') != string::npos) {
            while (timeStr.back() == '0') timeStr.pop_back();
            if (timeStr.back() == '.') timeStr.pop_back();
        }

        compressionratio = compressionPercentage;
        compressiontime = durationMs;
        string result = "";
        
        result += "\nOriginal file size    : " + to_string(originalSize) + " bytes\n";
        result += "Compressed file size  : " + to_string(compressedSize) + " bytes\n";
        result += "Compression time      : " + timeStr + " ms\n";
        result += "Compression percentage: " + ratioStr + " % saved \n";
        

        return result;
    }

    void huffmanFileCompress(const string& filename) {
        // Start timing execution
        auto startTime = chrono::high_resolution_clock::now();

        string text = readTextFile(filename);
        if (text.empty()) exit(1);

        int originalSize = static_cast<int>(text.size());
        unordered_map<char, int> freqResult = frequencyTable(text);

        Node* heap[256];
        int heapSize = 0;
        for (const auto& pair : freqResult) {
            Node* node  = new Node();
            node->ch    = pair.first;
            node->freq  = pair.second;
            heap[heapSize++] = node;
        }
        build_min_heap(heap, heapSize);

        Node* root = huffmanTree(heap, heapSize);

        unordered_map<char, string> codes;
        generateCodes(root, "", codes);

        string encoded = encode(text, codes);

        string newfilename = getNewFilename(filename, 'b');
        ofstream file(newfilename, ios::binary);
        if (!file.is_open()) {
            cout << "Error: cannot write to " << filename << endl;
            return;
        }
        serializeTree(root, file);
        packBits(encoded, file);
        file.close();

        // End timing execution
        auto endTime = chrono::high_resolution_clock::now();
        double durationMs = chrono::duration<double, milli>(endTime - startTime).count();

        // Check accurate compressed file size from disk
        int compressedSize = 0;
        ifstream compFile(newfilename, ios::binary | ios::ate);
        if (compFile.is_open()) {
            compressedSize = static_cast<int>(compFile.tellg());
            compFile.close();
        }

        stats = showStats(originalSize, compressedSize, durationMs);
    }

    string readCompressedFile(const string& filename, Node*& root) {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cout << "Error: cannot open " << filename << endl;
            return "";
        }
        root = deserializeTree(file);
        string encoded = unpackBits(file);
        file.close();
        return encoded;
    }

    void huffmanFileDecompress(const string& filename){
        Node* loadedRoot = nullptr;
        string loadedEncoded = readCompressedFile(filename, loadedRoot);
        const string& text = decode(loadedEncoded, loadedRoot);
        string newFileName = getNewFilename(filename, 't');
        ofstream outFile(newFileName);
        if (outFile.is_open()) {
            outFile << text;
            outFile.close();
            cout << "Decoded output saved " << endl;
        } else {
            cout << "Error : cannot write " << endl;
        }
    }

} // namespace Huffman