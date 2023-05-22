#ifndef PRINTERS_H
#define PRINTERS_H

#include "engine.h"
#include "utils.h"
#include "moves.h"

const char ascii_pieces[] = "PNBRQKpnbrqk-";


void printBitboard(uint32_t bitboard);

void printPVLine(std::vector<uint32_t> pv);

void printMoveDetailed(uint32_t move);

void printMove(uint32_t move);

void printMovesDetailed(Moves &moves);

void printMoveHeader();
// Function to print the chess board
void printBoard(ChessBoard& board);

std::string squaretoCoordinate(int square);

#endif