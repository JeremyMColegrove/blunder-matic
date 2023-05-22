#ifndef EVALUATION_H
#define EVALUATION_H

#include "engine.h"
#include "utils.h"
#include "logger.h"

#define flip(sq) ((sq)^56)

int getPieceSquareValue(int piece, int square);
int getPieceValue(int piece);
int evaluate(ChessBoard &board);

#endif