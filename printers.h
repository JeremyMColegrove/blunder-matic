#ifndef PRINTERS_H
#define PRINTERS_H

#include "engine.h"
#include "utils.h"


void print_bitboard(uint64_t bitboard);


void print_move(uint64_t move);

// Function to print the chess board
void print_board(ChessBoard& board);

#endif