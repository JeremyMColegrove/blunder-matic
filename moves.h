#ifndef MOVES_H
#define MOVES_H

#include "engine.h"
#include "utils.h"

constexpr int MAX_PLY = 64;
constexpr uint64_t NOT_A_FILE = 0xFEFEFEFEFEFEFEFE;
constexpr uint64_t NOT_H_FILE = 0x7F7F7F7F7F7F7F7F;
constexpr int BOARD_SIZE = 64;
constexpr int RANKS = 8;
constexpr int FILES = 8;
constexpr uint64_t LSB_MASK = 0x1;

struct Moves {
    int list[MAX_PLY];
    int count;
};



void generateMoves(ChessBoard &board, Moves *moves);


void initAttackTables();


#endif