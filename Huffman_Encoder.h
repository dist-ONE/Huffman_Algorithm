#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <filesystem>

#define BLOCK 4096 // 4KB blocks - read blocks of data from the file
#define ALPHABET 256 // ASCII + Extended ASCII.
#define MAGIC 0xDEADEAEF // 32-bit magic number.
#define MAX_CODE_SIZE (ALPHABET / 8) // Bytes for a maximum, 256-bit code.
#define MAX_TREE_SIZE (3 * ALPHABET - 1) // Maximum Huffman tree dump size

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

class Code {
private:
    uint32_t top;
    uint8_t bits[MAX_CODE_SIZE];

public:
    Code();

    bool push_bit(uint8_t bit);
    bool pop_bit(uint8_t& bit);
    bool get_bit(uint32_t index) const;
    uint32_t get_size() const;
};

struct NodeCompare {
    bool operator()(const Node* l, const Node* r) const {
        return l->get_frequency() > r->get_frequency();
    }
};

class Huffman_Encoder {
private:
    uint64_t histogram[ALPHABET] = {0};
    Code code_table[ALPHABET];
    uint16_t unique_symbols = 0;

    Node* node_join(Node* left, Node* right);
    void delete_tree(Node* root);
    void build_histogram(const string& infile);
    Node* build_tree();
    void build_code_table(Node* root, Code current_code);
    void write_tree_dump(Node* root, ofstream& out);
    void write_encoded_data(const string& infile, ofstream& out);


public:
    void encode(const string& infile, const string& outfile);
};