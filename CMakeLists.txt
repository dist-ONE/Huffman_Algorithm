cmake_minimum_required(VERSION 4.1)
project(Huffman_Encoding)

set(CMAKE_CXX_STANDARD 20)

add_executable(encoder
        encode.cpp
        Huffman_Encoder.cpp
        Huffman_Encoder.h
)

add_executable(decoder
        decode.cpp
        Huffman_Decoder.cpp
        Huffman_Decoder.h
)
