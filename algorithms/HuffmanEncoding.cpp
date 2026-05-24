#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
using namespace std;

struct Node{
    char ch;
    int freq;
    Node* left = nullptr;
    Node* right = nullptr;
};



// helper function to buid  MIN heap.
void heapify_min_heap(Node* array[], int size, int root) {

  // to calculate the left and right we have.
  int l = 2 * root + 1;
  int r = 2 * root + 2;

  // let the smallest as root at the start.
  int smallest = root;

  // update the smallest accordingly.
  // if left < smallest then smallest = left.
  if (l < size && array[l]->freq < array[smallest]->freq) {
    smallest = l;
  }
  // if right < smallest then smallest = right.
  if (r < size && array[r]->freq < array[smallest]->freq) {
    smallest = r;
  }
  // if smallest is not the root then heapify it.
  if (smallest != root) {
    // array[root] , array[smallest] = array[smallest] , array[root];
    Node* temp = array[root];
    array[root] = array[smallest];
    array[smallest] = temp;
    // Shift up function that heapify the other elements of heap-tress present above.
    heapify_min_heap(array, size, smallest); // recursion!!!
  }
}

// MIN Heap build fuction : It will use the `Shift DOWN` approach as we are using the reversed loop.
void build_min_heap(Node* array[],int size){
  //reversed loop for shift down.
  for (int i = size - 1; i >= 0; i--) {
    heapify_min_heap(array, size, i);//index is being passed : `i`
  }
}

//deletion function for min heap : It will use the `Shift Down` approach, starting from index 0.
Node* delete_min(Node * heap[] , int &size){
  if (size<=0) return nullptr;
  //Store the 0th element to return.
  Node* min_freq_node = heap[0];
  //swap last element with root
  heap[0] = heap[size-1];
  //remove last element
  size--;
  //passing 0 as root.
  heapify_min_heap(heap,size,0);
  
  return min_freq_node;
}



//function to make frequency table of characters in the text.
unordered_map<char, int> frequencyTable(string text) {
    unordered_map<char,int> freqMap;
    for(char c : text){
        freqMap[c]++;
    }
    return freqMap;
}


//function to make huffman tree.
Node* huffmanTree(Node* heap[],int &size) {
    while(size>1){
        Node* left = delete_min(heap,size);
        Node* right = delete_min(heap,size);
    
        Node* internal_node = new Node;
        internal_node->freq = left->freq+right->freq;
        internal_node->ch = '#';

        internal_node->left = left;
        internal_node->right = right;

        heap[size] = internal_node;
        size++;
        build_min_heap(heap,size);
    }
    //return head of huffman tree.
    return heap[0];
}

//assign binary to the each node in the huffman tree.
void generateCodes(Node* root, string currentCode, unordered_map<char, string>& codes) {
    //base case.
    if(root->left==nullptr && root->right == nullptr){
        codes[root->ch] = currentCode;
        return;
    }
    //recursive case.
    if(root->right != nullptr){ //right exists
        generateCodes(root->right,currentCode+"1",codes);
    }
    if(root->left != nullptr){ //left exists
        generateCodes(root->left,currentCode+"0",codes);
    }
}
//combine all the binaries of all the chars (against their specific binaries stored in map) in one string to replace the orignal text.
string encode(const string& text, unordered_map<char, string>& codes) {
    string result;
    for(char c : text){
        result += codes[c];
    }
    return result;
}

//decode the binary to text.
string decode(const string& encoded, Node* root) {
    string result;
    Node* current = root;
    for(char bit : encoded) {
        if(bit == '0'){
            current = current->left;
        }
        else if(bit=='1'){
            current = current->right;
        } 
        if(current->left==nullptr && current->right == nullptr){
            result +=current->ch;
            current = root;
        }
    }
    return result;
}

//function to read a file
string readTextFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: cannot open " << filename << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    file.close();
    return content;
}

// function to write a file.
void writeTextFile(const string& filename, const string& text){
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << text;
        outFile.close();
        cout << "Decoded output saved " << endl;
    } else {
        cout << "Error : cannot write " << endl;
    }
}

//serialize and deserilize the tree : // memory address // number of bytes to write
void serializeTree(Node* root, ofstream& file) {
    if (!root) return;
    if (!root->left && !root->right) {
        char flag = '1';
        file.write(&flag,sizeof(char));
        file.write(&root->ch,sizeof(char));
    } else {
        char flag = '0';
        file.write(&flag, sizeof(char));
        serializeTree(root->left,  file);
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
    internal->left  = deserializeTree(file);   // recurse left
    internal->right = deserializeTree(file);   // recurse right
    return internal;
}



// PACK actual bytes, also stores the original bit count so unpack, knows how many real bits are in the last byte
void packBits(const string& encoded, ofstream& file) {
    // store total number of bits (4 bytes)
    // needed during unpack to ignore padding in last byte
    int totalBits = encoded.size();
    file.write((char*)&totalBits, sizeof(int));

    int i = 0;
    while (i < (int)encoded.size()) {
        unsigned char byte = 0;

        // fill up to 8 bits into one byte
        for (int pos = 7; pos >= 0 && i < (int)encoded.size(); pos--, i++) {
            if (encoded[i] == '1') {
                byte = byte | (1 << pos);  // set this bit to 1
            }
            // if '0' — do nothing, bit is already 0
        }
        file.write((char*)&byte, sizeof(unsigned char));
    }
}

string unpackBits(ifstream& file) {
    // read total bit count we stored during packing
    int totalBits;
    file.read((char*)&totalBits, sizeof(int));

    string encoded = "";
    int bitsRead = 0;

    while (bitsRead < totalBits) {
        unsigned char byte;
        file.read((char*)&byte, sizeof(unsigned char));

        // extract bits from left to right (bit 7 down to bit 0)
        for (int pos = 7; pos >= 0 && bitsRead < totalBits; pos--, bitsRead++) {
            int bit = (byte >> pos) & 1;  // extract one bit
            encoded += (bit == 1) ? '1' : '0';
        }
    }
    return encoded;
}


//read and write compressed file
void writeCompressedFile(const string& filename, const string& encoded, Node* root) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Error: cannot write to " << filename << endl;
        return;
    }
    serializeTree(root, file);   // save the tree
    packBits(encoded, file);     // save actual compressed bits
    file.close();

    // show compression stats
    int origBits = encoded.size();        // bits before packing
    int packedBytes = (origBits + 7) / 8; // bytes after packing
    cout << "Huffman bits : " << origBits   << " bits" << endl;
    cout << "Packed into  : " << packedBytes << " bytes" << endl;
}

string readCompressedFile(const string& filename, Node*& root) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Error: cannot open " << filename << endl;
        return "";
    }
    root = deserializeTree(file);  // rebuild the tree
    string encoded = unpackBits(file);  // read bits back
    file.close();
    return encoded;
}


int main(){

    //READ INPUT FILE
    string text = readTextFile("./files/alice29.txt");
    if (text.empty()) return 1;

    //FREQUENCY TABLE 
    unordered_map<char,int> freqResult = frequencyTable(text);

    //BUILD MIN HEAP 
    Node* heap[256];
    int heapSize = 0;
    for (const auto& pair : freqResult) {
        Node* node  = new Node();
        node->ch    = pair.first;
        node->freq  = pair.second;
        heap[heapSize++] = node;
    }
    build_min_heap(heap, heapSize);

    //BUILD HUFFMAN TREE 
    Node* root = huffmanTree(heap, heapSize);

    //GENERATE CODES
    unordered_map<char, string> codes;
    generateCodes(root, "", codes);

    //ENCODE
    string encoded = encode(text, codes);

    //WRITE COMPRESSED FILE
    writeCompressedFile("./files/output.bin", encoded, root);

    //READ + DECODE
    Node* loadedRoot = nullptr;
    string loadedEncoded = readCompressedFile("./files/output.bin", loadedRoot);
    string decoded = decode(loadedEncoded, loadedRoot);

    //write text file
    writeTextFile("./files/output.txt",decoded);

    //Output Statistics
    cout << "\n======================================" << endl;
    cout << "Original size  : " << text.size()    << " bytes" << endl;
    cout << "Unique chars   : " << freqResult.size() << endl;
    cout << "Encoded bits   : " << encoded.size() << " bits"  << endl;
    cout << "Packed size    : " << (encoded.size()+7)/8 << " bytes" << endl;
    cout << "Compression    : " << (100.0 - ((encoded.size()/8.0) / text.size() * 100.0)) << " % saved " << endl;
    cout << "======================================" << endl;

    if (decoded == text)
    cout << "Round trip: SUCCESS " << endl;
    else
    cout << "Round trip: FAILED  " << endl;

    return 0;
}