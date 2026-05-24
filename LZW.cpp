#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <stdexcept>
using namespace std;

class Node {
public:
    string data;
    Node* next;
    Node(string value) {
        data = value;
        next = nullptr;
    }
};

class LinkedList {
private:
    Node* head;
    int count;

public:
    LinkedList() {
        head = nullptr;
        count = 0;
    }

    void push_back(string value) {
        Node* newNode = new Node(value);
        if (head == nullptr) {
            head = newNode;
            count++;
            return;
        }
        Node* temp = head;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = newNode;
        count++;
    }

    string get(int index) {
        if (index < 0 || index >= count) {
            throw out_of_range(
                "LinkedList::get — index " + to_string(index) +
                " is out of range (size = " + to_string(count) + ")"
            );
        }
        Node* temp = head;
        for (int i = 0; i < index; i++) {
            temp = temp->next;
        }
        return temp->data;
    }

    int size() {
        return count;
    }

    ~LinkedList() {
        Node* temp = head;
        while (temp != nullptr) {
            Node* next = temp->next;
            delete temp;
            temp = next;
        }
    }
};

void compress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in.is_open()) {
        cerr << "Error: cannot open input file: " << inputFile << endl;
        return;
    }
    ofstream out(outputFile, ios::binary);
    if (!out.is_open()) {
        cerr << "Error: cannot open output file: " << outputFile << endl;
        return;
    }

    string data((istreambuf_iterator<char>(in)),
                  istreambuf_iterator<char>());

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

    in.close();
    out.close();
}

void decompress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in.is_open()) {
        cerr << "Error: cannot open input file: " << inputFile << endl;
        return;
    }
    ofstream out(outputFile, ios::binary);
    if (!out.is_open()) {
        cerr << "Error: cannot open output file: " << outputFile << endl;
        return;
    }

    LinkedList dict;
    for (int i = 0; i < 256; i++) {
        dict.push_back(string(1, char(i)));
    }
    int dictSize = 256;

    int oldCode;
    if (!in.read(reinterpret_cast<char*>(&oldCode), sizeof(oldCode))) {
        cerr << "Error: compressed file is empty or unreadable." << endl;
        return;
    }
    if (oldCode < 0 || oldCode >= dictSize) {
        cerr << "Error: first code " << oldCode << " is out of range." << endl;
        return;
    }

    string current = dict.get(oldCode);
    out << current;

    int newCode;
    while (in.read(reinterpret_cast<char*>(&newCode), sizeof(newCode))) {
        string entry;
        if (newCode < dictSize) {
            entry = dict.get(newCode);
        } else if (newCode == dictSize) {
            entry = current + current[0];
        } else {
            cerr << "Error: invalid code " << newCode << " encountered." << endl;
            break;
        }

        out << entry;
        dict.push_back(current + entry[0]);
        dictSize++;
        current = entry;
    }

    in.close();
    out.close();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage:" << endl;
        cout << "  Compress:   ./sorting c input.txt compressed.bin" << endl;
        cout << "  Decompress: ./sorting d compressed.bin output.txt" << endl;
        return 1;
    }

    string mode       = argv[1];
    string inputFile  = argv[2];
    string outputFile = argv[3];

    if (mode == "c") {
        compress(inputFile, outputFile);
        cout << "Compressed:   " << inputFile << " -> " << outputFile << endl;
    } else if (mode == "d") {
        decompress(inputFile, outputFile);
        cout << "Decompressed: " << inputFile << " -> " << outputFile << endl;
    } else {
        cerr << "Unknown mode '" << mode << "'. Use 'c' to compress or 'd' to decompress." << endl;
        return 1;
    }

    return 0;
}
