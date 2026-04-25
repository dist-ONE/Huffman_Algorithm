#pragma once
#include <cstdint>
#include <string>
#include <stack>
#include <vector>
#include <fstream>
#include <filesystem>

#define BLOCK 4096 // 4KB blocks - read blocks of data from the file
#define MAGIC 0xDEADEAEF // 32-bit magic number

using namespace std;

struct Header {
    uint32_t magic;
    uint16_t tree_size;
    uint64_t file_size;
} __attribute__((packed));

class Node {
private:
    Node *left;
    Node *right;
    uint8_t symbol;
    uint64_t frequency;

public:
    Node(uint8_t sym, uint64_t freq) : left(nullptr), right(nullptr), symbol(sym), frequency(freq) {}

    Node* get_left() const;
    Node* get_right() const;
    uint8_t get_symbol() const;
    uint64_t get_frequency() const;

    void set_left(Node* l);
    void set_right(Node* r);

    bool is_leaf() const;
};

class Huffman_Decoder {
private:
    Node* node_join(Node* left, Node* right);
    void delete_tree(Node* root);

public:
    void decode(const string& infile, const string& outfile);
};