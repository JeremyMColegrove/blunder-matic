#ifndef PRINTERS_H
#define PRINTERS_H

#include "engine.h"
#include "utils.h"
#include "moves.h"


void printBitboard(uint64_t bitboard);


void printMove(uint64_t move);

void printMoves(Moves &moves);

void printMoveHeader();
// Function to print the chess board
void printBoard(ChessBoard& board);

#endif