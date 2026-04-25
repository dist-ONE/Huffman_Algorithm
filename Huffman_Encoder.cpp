#include "Huffman_Encoder.h"
#include <iostream>
using namespace std;

// Node functions

Node* Node::get_left() const {
    return left;
}

Node* Node::get_right() const {
    return right;
}

uint8_t Node::get_symbol() const {
    return symbol;
}

uint64_t Node::get_frequency() const {
    return frequency;
}

void Node::set_left(Node* l) {
    left = l;
}

void Node::set_right(Node* r) {
    right = r;
}

bool Node::is_leaf() const {
    return left == nullptr && right == nullptr;
}

// Code Functions

Code::Code() {
    top = 0;
    for (int i = 0; i < MAX_CODE_SIZE; ++i)
        bits[i] = 0;
}

bool Code::push_bit(uint8_t bit) {
    if (top >= 256) return false;

    if (bit)
        bits[top / 8] |= (1 << (top % 8));
    else
        bits[top / 8] &= ~(1 << (top % 8));
    top++;

    return true;
}

bool Code::pop_bit(uint8_t& bit) {
    if (top == 0) return false;

    top--;
    bit = (bits[top / 8] & (1 << (top % 8))) ? 1 : 0;

    return true;
}

bool Code::get_bit(uint32_t index) const {
    if (index >= top) return false;

    return (bits[index / 8] & (1 << (index % 8))) != 0;
}

uint32_t Code::get_size() const {
    return top;
}

// Encoder Functions

Node* Huffman_Encoder::node_join(Node* left, Node* right) {
    Node* parent = new Node('$', left->get_frequency() + right->get_frequency());

    parent->set_left(left);
    parent->set_right(right);

    return parent;
}

void Huffman_Encoder::delete_tree(Node* root) {
    if (!root) return;

    delete_tree(root->get_left());
    delete_tree(root->get_right());

    delete root;
}

void Huffman_Encoder::build_histogram(const string& infile) {
    for (int i = 0; i < ALPHABET; i++) histogram[i] = 0;

    ifstream in(infile, ios::binary);
    if (!in.is_open()) return;

    uint8_t buffer[BLOCK];
    while (in.read(reinterpret_cast<char*>(buffer), BLOCK) || in.gcount() > 0) {
        for (streamsize i = 0; i < in.gcount(); ++i) {
            histogram[buffer[i]]++;
        }
    }
    in.close();

    histogram[0]++;
    histogram[255]++;

    unique_symbols = 0;
    for (int i = 0; i < ALPHABET; i++) {
        if (histogram[i] > 0) unique_symbols++;
    }
}

Node* Huffman_Encoder::build_tree() {
    priority_queue<Node*, vector<Node*>, NodeCompare> pq;

    for (int i = 0; i < ALPHABET; ++i) {
        if (histogram[i] > 0) {
            pq.push(new Node(static_cast<uint8_t>(i), histogram[i]));
        }
    }

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        pq.push(node_join(left, right));
    }

    return pq.empty() ? nullptr : pq.top();
}

void Huffman_Encoder::build_code_table(Node* root, Code current_code) {
    if (!root) return;

    if (root->is_leaf()) {
        code_table[root->get_symbol()] = current_code;
        return;
    }

    current_code.push_bit(0);
    build_code_table(root->get_left(), current_code);

    uint8_t bit;
    current_code.pop_bit(bit);
    current_code.push_bit(1);
    build_code_table(root->get_right(), current_code);
}

void Huffman_Encoder::write_tree_dump(Node* root, ofstream& out) {
    if (!root) return;

    write_tree_dump(root->get_left(), out);
    write_tree_dump(root->get_right(), out);

    if (root->is_leaf()) {
        out.put('L');
        out.put(static_cast<char>(root->get_symbol()));
    } else {
        out.put('I');
    }
}

void Huffman_Encoder::write_encoded_data(const string& infile, ofstream& out) {
    ifstream in(infile, ios::binary);
    if (!in.is_open()) return;

    uint8_t buffer[BLOCK];
    uint8_t out_byte = 0;
    int bit_count = 0;

    while (in.read(reinterpret_cast<char*>(buffer), BLOCK) || in.gcount() > 0) {
        for (streamsize i = 0; i < in.gcount(); ++i) {
            uint8_t sym = buffer[i];
            Code c = code_table[sym];

            for (uint32_t b = 0; b < c.get_size(); ++b) {
                uint8_t bit_val = c.get_bit(b) ? 1 : 0;

                if (bit_val)
                    out_byte |= (1 << bit_count);
                else
                    out_byte &= ~(1 << bit_count);

                bit_count++;

                if (bit_count == 8) {
                    out.put(static_cast<char>(out_byte));
                    out_byte = 0;
                    bit_count = 0;
                }
            }
        }
    }

    if (bit_count > 0) {
        out.put(static_cast<char>(out_byte));
    }

    in.close();
}

void Huffman_Encoder::encode(const string& inpath, const string& outpath) {
    build_histogram(inpath);

    Node* root = build_tree();

    Code empty_code;
    build_code_table(root, empty_code);

    Header h;
    h.magic = MAGIC;
    h.tree_size = (3 * unique_symbols) - 1;
    h.file_size = filesystem::file_size(inpath);

    ofstream out(outpath, ios::binary);
    if (!out.is_open()) {
        cerr << "Error opening output file." << endl;
        delete_tree(root);
        return;
    }

    out.write(reinterpret_cast<const char*>(&h), sizeof(Header));

    write_tree_dump(root, out);

    write_encoded_data(inpath, out);

    out.close();

    delete_tree(root);
}