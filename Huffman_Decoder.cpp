#include "Huffman_Decoder.h"
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

// Decoder Functions

Node* Huffman_Decoder::node_join(Node* left, Node* right) {
    Node* parent = new Node('$', 0);

    parent->set_left(left);
    parent->set_right(right);

    return parent;
}

void Huffman_Decoder::delete_tree(Node* root) {
    if (!root) return;

    delete_tree(root->get_left());
    delete_tree(root->get_right());

    delete root;
}

void Huffman_Decoder::decode(const string& inpath, const string& outpath) {
    ifstream in(inpath, ios::binary);
    if (!in.is_open()) {
        cerr << "Error opening input file." << endl;
        return;
    }

    Header h;
    in.read(reinterpret_cast<char*>(&h), sizeof(Header));

    if (h.magic != MAGIC) {
        cerr << "Error: Invalid file format. Magic number does not match." << endl;
        in.close();
        return;
    }

    vector<uint8_t> tree_dump(h.tree_size);
    in.read(reinterpret_cast<char*>(tree_dump.data()), h.tree_size);

    stack<Node*> st;
    for (int i = 0; i < h.tree_size; ++i) {
        if (tree_dump[i] == 'L') {
            i++;
            st.push(new Node(tree_dump[i], 0));

        } else if (tree_dump[i] == 'I') {
            Node* right = st.top(); st.pop();
            Node* left = st.top(); st.pop();
            st.push(node_join(left, right));
        }
    }

    if (st.empty()) {
        cerr << "Error: Failed to reconstruct Huffman tree." << endl;
        in.close();
        return;
    }

    Node* root = st.top();
    st.pop();

    ofstream out(outpath, ios::binary);
    if (!out.is_open()) {
        cerr << "Error opening output file." << endl;
        delete_tree(root);
        in.close();
        return;
    }

    uint64_t decoded_symbols = 0;
    Node* current_node = root;
    uint8_t buffer[BLOCK];

    while (decoded_symbols < h.file_size && (in.read(reinterpret_cast<char*>(buffer), BLOCK) || in.gcount() > 0)) {
        for (streamsize i = 0; i < in.gcount() && decoded_symbols < h.file_size; ++i) {
            uint8_t byte = buffer[i];

            for (int b = 0; b < 8 && decoded_symbols < h.file_size; ++b) {
                bool bit_val = (byte & (1 << b)) != 0;

                if (bit_val) {
                    current_node = current_node->get_right();
                } else {
                    current_node = current_node->get_left();
                }

                if (current_node->is_leaf()) {
                    out.put(static_cast<char>(current_node->get_symbol()));
                    decoded_symbols++;
                    current_node = root;
                }
            }
        }
    }

    in.close();
    out.close();

    delete_tree(root);
}