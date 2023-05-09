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

struct Bitboard {
    U64 pawns;
    U64 knights;
    U64 bishops;
    U64 rooks;
    U64 queens;
    U64 kings;
};

struct ChessBoard {
    Bitboard white_pieces;
    Bitboard black_pieces;
    bool white_to_move;
    uint8_t castling_rights; // 1 = white kingside, 2 = white queenside, 4 = black kingside, 8 = black queenside
    int en_passant_square;
    unsigned half_move_counter;
    unsigned full_move_counter;
};

#endif