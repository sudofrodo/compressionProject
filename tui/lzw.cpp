#include "lzw.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <chrono>

using namespace std;

namespace LZW {
    double compressionratio = 0.0;
    int compressiontime = 0;

    double GetCompressionRatio() {
    return compressionratio;
    }

    int GetCompressionTime(){
    return compressiontime;
    }

    string stats = "";

    Node::Node(string value) {
        data = value;
        next = NULL;
    }

    LinkedList::LinkedList() {
        head = NULL;
    }

    void LinkedList::push_back(string value) {
        Node* newNode = new Node(value);

        if (head == NULL) {
            head = newNode;
            return;
        }

        Node* temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }

    string LinkedList::get(int index) {
        Node* temp = head;
        for (int i = 0; i < index; i++) {
            temp = temp->next;
        }
        return temp->data;
    }

    int LinkedList::size() {
        int count = 0;
        Node* temp = head;
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
        return count;
    }

    string getNewFilename(string filename, char type){
        string baseName = filename;
        if(type == 't') {
            size_t dash = filename.find_last_of("-");
            if (dash != string::npos) {
                baseName = filename.substr(0,dash) + ".txt";
            }
        } else {
            size_t lastDot = filename.find_last_of(".");
            if (lastDot != string::npos) {
                baseName = filename.substr(0, lastDot) + "-lzwa.bin";
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
        compressionratio = compressionPercentage;
        compressiontime = durationMs;
        if (ratioStr.find('.') != string::npos) {
            while (ratioStr.back() == '0') ratioStr.pop_back();
            if (ratioStr.back() == '.') ratioStr.pop_back();
        }

        string timeStr = to_string(durationMs);
        if (timeStr.find('.') != string::npos) {
            while (timeStr.back() == '0') timeStr.pop_back();
            if (timeStr.back() == '.') timeStr.pop_back();
        }

        string result = "";
        
        result += "\nOriginal file size    : " + to_string(originalSize) + " bytes\n";
        result += "Compressed file size  : " + to_string(compressedSize) + " bytes\n";
        result += "Compression time      : " + timeStr + " ms\n";
        result += "Compression percentage: " + ratioStr + " % saved \n";
       
        return result;
    }

    
    
    void compress(const string& inputFile) {
        // Start timing execution
        auto startTime = chrono::high_resolution_clock::now();

        ifstream in(inputFile, ios::binary);
        if (!in.is_open()) {
            cout << "Error: cannot open " << inputFile << endl;
            return;
        }

        string outputFile = getNewFilename(inputFile, 'b');
        ofstream out(outputFile, ios::binary);

        string data((istreambuf_iterator<char>(in)),
                     istreambuf_iterator<char>());
        in.close();

        int originalSize = static_cast<int>(data.size());
        unordered_map<string, int> dict;

        for (int i = 0; i < 256; i++) {
            dict[string(1, char(i))] = i;
        }

        string current;
        int code = 256;

        for (char c : data) {
            string next = current + c;

            if (dict.count(next)) {
                current = next;
            } else {
                int value = dict[current];
                out.write(reinterpret_cast<char*>(&value), sizeof(value));
                dict[next] = code++;
                current = string(1, c);
            }
        }

        if (!current.empty()) {
            int value = dict[current];
            out.write(reinterpret_cast<char*>(&value), sizeof(value));
        }
        out.close();

        // End timing execution
        auto endTime = chrono::high_resolution_clock::now();
        double durationMs = chrono::duration<double, milli>(endTime - startTime).count();

        // Check accurate compressed file size from disk
        int compressedSize = 0;
        ifstream compFile(outputFile, ios::binary | ios::ate);
        if (compFile.is_open()) {
            compressedSize = static_cast<int>(compFile.tellg());
            compFile.close();
        }

        stats = showStats(originalSize, compressedSize, durationMs);
    }

    void decompress(const string& inputFile) {
        ifstream in(inputFile, ios::binary);

        string outputFile = getNewFilename(inputFile, 't');
        ofstream out(outputFile, ios::binary);

        LinkedList dict;

        for (int i = 0; i < 256; i++) {
            dict.push_back(string(1, char(i)));
        }

        int oldCode, newCode;

        if (in.read(reinterpret_cast<char*>(&oldCode), sizeof(oldCode))) {
            string current = dict.get(oldCode);
            out << current;

            while (in.read(reinterpret_cast<char*>(&newCode), sizeof(newCode))) {
                string entry;

                if (newCode < dict.size()) {
                    entry = dict.get(newCode);
                } else {
                    entry = current + current[0];
                }

                out << entry;
                dict.push_back(current + entry[0]);
                current = entry;
            }
        }

        in.close();
        out.close();
    }

} // namespace LZW