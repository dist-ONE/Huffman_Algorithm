#include <iostream>
#include <unistd.h>
#include <filesystem>
#include <iomanip>
#include "Huffman_Decoder.h"

using namespace std;

void print_help() {
    cout << "Usage: decode [-h] [-i infile] [-o outfile] [-s]\n"
         << "  -h          Prints out a help message describing the purpose of the program.\n"
         << "  -i infile   Specifies the input file to decode.\n"
         << "  -o outfile  Specifies the output file to write decompressed input to.\n"
         << "  -s          Prints decompression statistics.\n";
}

int main(int argc, char** argv) {
    int opt;
    string infile = "../Test_Files/encoded_text.txt";
    string outfile = "../Test_Files/decoded_text.txt";
    bool print_stats = false;

    while ((opt = getopt(argc, argv, "hi:o:s")) != -1) {
        switch (opt) {
            case 'h': 
                print_help();
                return 0;
            case 'i': 
                infile = optarg;
                break;
            case 'o': 
                outfile = optarg;
                break;
            case 's': 
                print_stats = true;
                break;
            default: 
                print_help(); 
                return 1;
        }
    }

    Huffman_Decoder decoder;
    decoder.decode(infile, outfile);

    if (print_stats) {
        try {
            uint64_t compressed_size = filesystem::file_size(infile);
            uint64_t decompressed_size = filesystem::file_size(outfile);
            double space_saving = 100.0 * (1.0 - (static_cast<double>(compressed_size) / decompressed_size));
            
            cout << "Compressed file size: " << compressed_size << " bytes\n";
            cout << "Decompressed file size: " << decompressed_size << " bytes\n";
            cout << fixed << setprecision(2);
            cout << "Space saving: " << space_saving << "%\n";
        } catch (const filesystem::filesystem_error& e) {
            cout << "Failed to calculate: " << e.what() << endl;
        }
    }

    return 0;
}