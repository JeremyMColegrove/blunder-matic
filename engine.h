#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>
#include <array>

#define U64 u_int64_t

using namespace std;

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_square
};

const int castling_rights[64] = {
            7, 15, 15, 15,  3, 15, 15, 11,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            13, 15, 15, 15, 12, 15, 15, 14
    };

//Encode the pieces here as they would be output to the screen
enum {P, N, B, R, Q, K, p, n, b, r, q, k, no_piece};

enum {white, black, both};

struct ChessBoard {
    U64 bitboards[12];
    U64 occupancies[3];
    bool white_to_move;
    uint8_t castling_rights; // 1 = white kingside, 2 = white queenside, 4 = black kingside, 8 = black queenside
    int en_passant_square;
    unsigned half_move_counter;
    unsigned full_move_counter;
};


#endif