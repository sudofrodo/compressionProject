#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

using namespace std;

class Node {
public:
    string data;
    Node* next;

    Node(string value) {
        data = value;
        next = NULL;
    }
};

class LinkedList {
private:
    Node* head;

public:
    LinkedList() {
        head = NULL;
    }

    void push_back(string value) {
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

    string get(int index) {
        Node* temp = head;

        for (int i = 0; i < index; i++) {
            temp = temp->next;
        }

        return temp->data;
    }

    int size() {
        int count = 0;

        Node* temp = head;

        while (temp != NULL) {
            count++;
            temp = temp->next;
        }

        return count;
    }
};

void compress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);

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
    ofstream out(outputFile, ios::binary);

    LinkedList dict;

    for (int i = 0; i < 256; i++) {
        dict.push_back(string(1, char(i)));
    }

    int oldCode, newCode;

    in.read(reinterpret_cast<char*>(&oldCode), sizeof(oldCode));

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

    in.close();
    out.close();
}
int main() {

    compress("lorem.txt", "compressed.lzw");

    decompress("compressed.lzw", "output.txt");

    return 0;
}