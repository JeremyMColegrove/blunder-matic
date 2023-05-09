#ifndef MOVES_H
#define MOVES_H

#include "engine.h"
#include "utils.h"

constexpr int MAX_PLY = 64;
constexpr uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFE;
constexpr uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7F;

struct Moves {
    int list[MAX_PLY];
    int count;
};



void generate_moves(ChessBoard &board, Moves *moves);


void init_moves();


#endif