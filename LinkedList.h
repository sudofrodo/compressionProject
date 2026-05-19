#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cstdint>
#include <iostream>
#include <string>

// The triplet structure for LZ77

struct lz77_triplet {
    uint16_t offset; //how much jump back. (2 bytes)
    uint8_t length; //length of the matched subtring (short because english words dont have that much length) (1 bytes)
    char nextChar; //character after the matched subtring

    //initiazation list
    lz77_triplet(uint16_t offset , uint8_t length, char nextChar) : offset(offset) , length(length), nextChar(nextChar) {}; 
    lz77_triplet() {
        offset = 0;
        length = 0;
        nextChar = '\0';
    }
};


class LinkedList {
public:
    struct Node {
        lz77_triplet data;
        Node* next;
        Node(lz77_triplet val) : data(val), next(nullptr) {}
    };

    Node* head;
    Node* tail; // tail for O(1) insertion
    int listSize;

public:
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}


    // Specialized insert for triplets
    void insert(int offset, int length, char nextChar) {
        lz77_triplet newTriplet(offset, length, nextChar);
        Node* newNode = new Node(newTriplet);
        
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }

    void insert(lz77_triplet newTriplet) {
        Node* newNode = new Node(newTriplet);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }
    
    
    int size(){
        return listSize;
    }

    // helper for decompression to get the head node
    Node* getHead() {
        return head;
    }

    
};

#endif