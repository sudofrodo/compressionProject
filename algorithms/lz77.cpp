#include <iostream>
#include <fstream>
#include <cstdint>
using namespace std;
#include "LinkedList.h"

const int MAX_WINDOW_SIZE = 1024; //maximum size of search window

LinkedList lz77Compress(string text) {

    LinkedList ledger;
    int i = 0; //starting index of look-ahead window
    while(i < text.size()) {
        //maxmatchlength to find out the largest matched substring in the search buffer.
        uint16_t maxMatchDistance = 0;
        uint8_t maxMatchLength = 0; 
        int startSearch = (i < MAX_WINDOW_SIZE) ? 0 : i - MAX_WINDOW_SIZE;

        for (int j = startSearch; j < i ; j++) {
            uint8_t matchLength = 0;
            //adding matchlength to save us from infinite loop and add consistency in matching.
            while(text[i + matchLength] == text[j + matchLength]) {
                matchLength++;
                if(i + matchLength >= text.size() || matchLength >= 255){
                    break;
                }
            }
            if(matchLength > maxMatchLength) {
                maxMatchLength = matchLength;
                maxMatchDistance = i - j; //this is the jump ,
            }
        }
        //we found no match , hence just pack a literal in struct
        if(maxMatchLength == 0) {
            maxMatchDistance = 0;
            maxMatchLength = 0;
        }
        
        if (maxMatchLength >= 5) {
            char nextChar = (i + maxMatchLength < text.size()) ? text[i + maxMatchLength] : '\0';
            ledger.insert(maxMatchDistance, maxMatchLength, nextChar);
            i += maxMatchLength + 1;
        } else {
            ledger.insert(0, 0, text[i]);
            i++;
        }
        
    }
    return ledger;
}


string lz77Decompress(LinkedList ledger) {
   string output;
   LinkedList::Node* curr = ledger.getHead();
   while(curr != nullptr) {
    lz77_triplet tri = curr -> data;
        if(tri.length > 0) {
            int startPos = output.size() - tri.offset;
            for (int i = 0; i < tri.length; i++) {
                output += output[startPos + i];
            }
        }
        //one single literal that had no match
        output += tri.nextChar;
    curr = curr -> next;
   }
   
   return output;
}


//for debugging.
string tripletsToString(LinkedList ledger){
    string text = "";
    LinkedList::Node* curr = ledger.getHead();
    while(curr != nullptr) {
    lz77_triplet tri = curr -> data;
    text.append("< " + to_string(tri.offset) + ", " + to_string(tri.length) + ", ");
    if(tri.nextChar == '\0'){
        text += '-';
        text += ">\n";
    } else {
        text += tri.nextChar;
        text += ">\n";
    }
    curr = curr -> next;
   }
   return text;
}

//helper in compression.
LinkedList textfileToTriplets(ifstream &inFile) {
    string inputStr, line;
    if(inFile.is_open()) {
        while(getline(inFile, line)) {
            inputStr += line + "\n";
        }
    } else {
        cerr << "File cannot be opened." << endl;
    }
    inFile.close();
    
    return lz77Compress(inputStr);
}

//helper in decompression.
LinkedList binaryFileToTriplets(ifstream &inFile) {
    LinkedList input;
    lz77_triplet tri;
    
    if(inFile.is_open()) {
    while(inFile.read(reinterpret_cast<char*>(&tri.offset), sizeof(tri.offset)) &&
          inFile.read(reinterpret_cast<char*>(&tri.length), sizeof(tri.length)) &&
          inFile.read(&tri.nextChar, sizeof(tri.nextChar))) {
            
        input.insert(tri);
    } } else {
        cerr << "File cannot be opened." << endl;
    }
    inFile.close();
    return input;
}

//to cut file extension from filenames
string getNewFilename(string filename, char type){
    string baseName = filename;
    size_t lastDot = filename.find_last_of(".");
    if (lastDot != string::npos) {
        baseName = filename.substr(0, lastDot);
    }
    if(type == 't') {
        baseName = baseName + ".txt";
    } else {
        baseName = baseName + ".bin";
    }
    return baseName;
}

void compressToBinaryFile(string filename){
    ifstream inFile(filename);

    if(inFile.is_open()) {
    LinkedList ledger = textfileToTriplets(inFile);

    string binFileName = getNewFilename(filename,'b');
    ofstream outFile(binFileName, ios::binary);

    if(outFile.is_open()) {
    LinkedList::Node* curr = ledger.getHead();
    while(curr != nullptr) {
    lz77_triplet tri = curr -> data;
    outFile.write(reinterpret_cast<char*>(&tri.offset), sizeof(tri.offset));
    outFile.write(reinterpret_cast<char*>(&tri.length), sizeof(tri.length));
    outFile.write(&tri.nextChar, sizeof(tri.nextChar));
    curr = curr -> next;
    }
    cout << "Compressed successfully.";
    } else {
        cerr << "There's an error creating a binary file." << endl;
    }
    outFile.close();
    } else {
        cout << "File cannot be opened." << endl;
    }
    inFile.close();
}

void decompressToTextFile(string filename){
    ifstream infile(filename);
    if(infile.is_open()){
        string decompressed = lz77Decompress(binaryFileToTriplets(infile));

        string textFilename = getNewFilename(filename , 't');
        ofstream outFile(textFilename);
        if(outFile.is_open()) {
        outFile << decompressed;
        cout << "File decompressed Successfully.";
        } else {
        cerr << "Error : Cannot decompress file." << endl;
        } 
        outFile.close();
    } else {
        cerr << "File cannot be opened." << endl;
    }
    infile.close();
}

int main() {
   
   compressToBinaryFile("./files/widget_demo.cpp");
   //decompressToTextFile("lorem.bin");
   
    cout << endl;
    return 0;
}